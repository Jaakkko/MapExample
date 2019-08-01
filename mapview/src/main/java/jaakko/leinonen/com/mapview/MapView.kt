package jaakko.leinonen.com.mapview

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.opengl.EGL14
import android.os.Build
import android.util.AttributeSet
import android.view.Choreographer
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.TextView
import androidx.core.view.ViewCompat
import jaakko.leinonen.com.loading.TileDownloader
import jaakko.leinonen.com.loading.TileLoader
import jaakko.leinonen.com.touchgestures.TouchGestureDetector
import java.lang.Math.cos
import java.lang.Math.sin
import java.util.concurrent.locks.Condition
import java.util.concurrent.locks.ReentrantLock
import javax.microedition.khronos.egl.*
import javax.microedition.khronos.egl.EGL10.*
import kotlin.concurrent.thread
import kotlin.concurrent.withLock

class MapView : SurfaceView {
    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    private var mHandle: Long = 0

    private lateinit var mEGL: EGL10
    private lateinit var mEGLDisplay: EGLDisplay
    private lateinit var mEGLContext: EGLContext
    private lateinit var mEGLSurface: EGLSurface
    private lateinit var mEGLConfig: EGLConfig

    private var mMapRunning: Boolean = false

    private lateinit var mTileLoader: TileLoader

    private val viewPort: ViewPort = ViewPort()

    val renderLock = ReentrantLock()
    val renderCondition: Condition = renderLock.newCondition()

    private val mTouchGestureDetector: TouchGestureDetector = TouchGestureDetector(context, object : TouchGestureDetector.SimpleTouchGestureListener() {
        override fun onMove(detector: TouchGestureDetector, deltaX: Float, deltaY: Float) {
            val dx: Double = deltaX / width * viewPort.width
            val dy: Double = deltaY / height * -viewPort.height
            if (viewPort.translate(dx, dy)) {
                detector.stopScroller()
            }
        }
        override fun onScale(detector: TouchGestureDetector, focusX: Float, focusY: Float, scaleFactor: Float) {
            if (scaleFactor < 1.0f && (viewPort.width >= viewPort.maxWidth || viewPort.height >= viewPort.maxWidth)) {
                viewPort.s()
                return
            }
            if (scaleFactor > 1.0f && (viewPort.width <= ViewPort.minWidth)) {
                viewPort.s()
                return
            }


            val hw: Double = viewPort.width * .5f
            val hh: Double = viewPort.height * .5f

            val c: Double = cos(-viewPort.angle)
            val s: Double = sin(-viewPort.angle)

            val fx: Double = map(focusX.toDouble(), 0.0, width.toDouble(), viewPort.centerX + hw, viewPort.centerX - hw)
            val fy: Double = map(focusY.toDouble(), 0.0, height.toDouble(), viewPort.centerY - hh, viewPort.centerY + hh)

            val x: Double = c * (fx - viewPort.centerX) - s * (fy - viewPort.centerY) + viewPort.centerX
            val y: Double = s * (fx - viewPort.centerX) + c * (fy - viewPort.centerY) + viewPort.centerY

            if (viewPort.scale(x, y, scaleFactor.toDouble())) {
                detector.stopScroller()
            }
        }
        override fun onRotate(focusX: Float, focusY: Float, angle: Float) {
            val c: Double = cos(-viewPort.angle)
            val s: Double = sin(-viewPort.angle)

            val hw: Double = viewPort.width * .5f
            val hh: Double = viewPort.height * .5f

            val fx: Double = map(focusX.toDouble(), 0.0, width.toDouble(), viewPort.centerX - hw, viewPort.centerX + hw)
            val fy: Double = map(focusY.toDouble(), 0.0, height.toDouble(), viewPort.centerY + hh, viewPort.centerY - hh)

            val x: Double = c * (fx - viewPort.centerX) - s * (fy - viewPort.centerY) + viewPort.centerX
            val y: Double = s * (fx - viewPort.centerX) + c * (fy - viewPort.centerY) + viewPort.centerY

            viewPort.rotate(x, y, -angle.toDouble())
        }
    })

    lateinit var mapUrls: Array<TileDownloader.LoadURL>

    constructor(context: Context?) : super(context)
    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs)
    constructor(context: Context?, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr)

    init {
        setWillNotDraw(false)

        var surfaceCreated = false
        var surfaceDestroyed = false
        var surfaceChanged = false

        thread {
            while (true) {
                if (surfaceDestroyed) {
                    surfaceDestroyed = false

                    mEGL.eglDestroySurface(mEGLDisplay, mEGLSurface)
                    mEGL.eglDestroyContext(mEGLDisplay, mEGLContext)

                    mMapRunning = false

                    cleanUp(mHandle)
                }
                if (surfaceCreated) {
                    surfaceCreated = false

                    mEGL = EGLContext.getEGL() as EGL10
                    mEGLDisplay = mEGL.eglGetDisplay(EGL_DEFAULT_DISPLAY)
                    mEGLConfig = run {
                        val attributeList: IntArray = if (Build.VERSION.SDK_INT >= 17) intArrayOf(
                            EGL_RED_SIZE, 8,
                            EGL_GREEN_SIZE, 8,
                            EGL_BLUE_SIZE, 8,
                            EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                            EGL_NONE
                        )
                        else intArrayOf(
                            EGL_RED_SIZE, 8,
                            EGL_GREEN_SIZE, 8,
                            EGL_BLUE_SIZE, 8,
                            EGL_NONE
                        )
                        val configs: Array<EGLConfig?> = arrayOfNulls(1)
                        val numConfig = IntArray(1)

                        mEGL.eglChooseConfig(mEGLDisplay, attributeList, configs, 1, numConfig)

                        configs[0]!!
                    }
                    mEGLContext = mEGL.eglCreateContext(mEGLDisplay, mEGLConfig, EGL_NO_CONTEXT, if (Build.VERSION.SDK_INT >= 17) intArrayOf(EGL14.EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE) else intArrayOf(EGL_NONE))
                    mEGLSurface = mEGL.eglCreateWindowSurface(mEGLDisplay, mEGLConfig, holder, intArrayOf(EGL_NONE))
                    mEGL.eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext)

                    mHandle = initialize(1)

                    mMapRunning = true

                    postInvalidate()
                }
                if (surfaceChanged) {
                    surfaceChanged = false

                    screenSizeChanged(mHandle, width, height)
                }

                if (mMapRunning) {
                    render(mHandle)

                    mEGL.eglSwapBuffers(mEGLDisplay, mEGLSurface)


                    // Upload tiles to GPU
                    // Upload if no animation running and less than 3 uploaded or no touch down
                    if (!mTouchGestureDetector.animationRunning) {
                        prepareToUpload(mHandle)
                        synchronized(mTileLoader.loadLock) {
                            var c = 0
                            while (mTileLoader.loaded.isNotEmpty() && (c++ < 3 || !mTouchGestureDetector.touchDown)) {
                                val load: TileLoader.Load = mTileLoader.loaded.remove()
                                upload(mHandle, load.index, load.bitmap, load.haveTile, load.alpha)
                                load.bitmap?.recycle()
                            }

                            if (c != 0) {
                                ViewCompat.postInvalidateOnAnimation(this)
                            }
                        }
                        finishUploading(mHandle)
                    }
                }

                renderLock.withLock {
                    renderCondition.await()
                }
            }
        }

        holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
                surfaceChanged = true
                renderLock.withLock {
                    renderCondition.signal()
                }
            }

            override fun surfaceDestroyed(holder: SurfaceHolder?) {
                surfaceDestroyed = true
                renderLock.withLock {
                    renderCondition.signal()
                }
            }

            override fun surfaceCreated(holder: SurfaceHolder?) {
                mTileLoader = TileLoader(context, mapUrls)

                surfaceCreated = true
                renderLock.withLock {
                    renderCondition.signal()
                }
            }
        })
    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)

        // TODO: Händlää täällä tallennettu viewPort

        if (w > h) {
            viewPort.width = viewPort.maxWidth
            viewPort.height = viewPort.maxWidth * h / w
        }
        else {
            viewPort.width = viewPort.maxWidth * w / h
            viewPort.height = viewPort.maxWidth
        }
    }

    override fun onDraw(canvas: Canvas?) {
        if (mMapRunning) {
            draw(mHandle, viewPort.centerX, viewPort.centerY, viewPort.width, viewPort.height, viewPort.angle)

            renderLock.withLock {
                renderCondition.signal()
            }
        }

        // logFrames()

        postInvalidate()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean = mTouchGestureDetector.onTouchEvent(event).apply {
        if (this) ViewCompat.postInvalidateOnAnimation(this@MapView)
    }

    override fun computeScroll() {
        if (mTouchGestureDetector.computeScroll()) {
            ViewCompat.postInvalidateOnAnimation(this)
        }
    }



    /***
     * =======================================
     * Native
     * =======================================
     */

    /**
     * ---------------------------------------
     * UI Thread
     * ---------------------------------------
     */

    fun get(loadIndex: Int, tileIndex: Long, layer: Int, row: Int, col: Int): Boolean {
        if (mTouchGestureDetector.animationRunning)
            return false

        val tileLoader: TileLoader = mTileLoader
        tileLoader.runInBackground {
            val load: TileLoader.Load = tileLoader.load(loadIndex, tileIndex, layer, row, col)
            if (mMapRunning) {
                tileLoader.add(load)
                postInvalidate()
            }
        }

        return true
    }


    private external fun draw(handle: Long, viewPortCenterX: Double, viewPortCenterY: Double, viewPortWidth: Double, viewPortHeight: Double, viewPortAngle: Double)


    /**
     * ---------------------------------------
     * GL Thread
     * ---------------------------------------
     */

    /**
     * @param mapsCount
     */
    private external fun initialize(mapsCount: Int): Long
    private external fun screenSizeChanged(handle: Long, width: Int, height: Int)

    private external fun render(handle: Long)
    private external fun cleanUp(handle: Long)

    private external fun prepareToUpload(handle: Long)
    private external fun upload(handle: Long, tileIndex: Long, bitmap: Bitmap?, tileExists: Boolean, alpha: Float)
    private external fun finishUploading(handle: Long)
}