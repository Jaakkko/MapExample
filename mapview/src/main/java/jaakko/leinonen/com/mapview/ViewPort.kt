package jaakko.leinonen.com.mapview

import java.lang.Math.*

class ViewPort {
    companion object {
        // const val minWidth: Double = 0.00012207031
        const val minWidth: Double = 0.00000095367431640625 // 2/2^21
    }

    var maxWidth: Double = 2.0

    private var onViewPortChangedListener: OnViewPortChangedListener? = null

    var centerX: Double = 0.0
    var centerY: Double = 0.0
    var width: Double = 0.0
    var height: Double = 0.0
    var angle: Double = 0.0; private set

    val left: Double
        get() {
            val c: Double = cos(angle)
            val s: Double = sin(angle)

            val p1: Double = .5f * (-width * c - height * s)
            val p2: Double = .5f * (width * c - height * s)
            val p3: Double = .5f * (width * c + height * s)
            val p4: Double = .5f * (-width * c + height * s)

            return centerX + doubleArrayOf(p1, p2, p3, p4).min()!!
        }
    val right: Double
        get() {
            val c: Double = cos(angle)
            val s: Double = sin(angle)

            val p1: Double = .5f * (-width * c - height * s)
            val p2: Double = .5f * (width * c - height * s)
            val p3: Double = .5f * (width * c + height * s)
            val p4: Double = .5f * (-width * c + height * s)

            return centerX + doubleArrayOf(p1, p2, p3, p4).max()!!
        }
    val bottom: Double
        get() {
            val c: Double = cos(angle)
            val s: Double = sin(angle)

            val p1: Double = .5f * (-width * s + height * c)
            val p2: Double = .5f * (width * s + height * c)
            val p3: Double = .5f * (width * s - height * c)
            val p4: Double = .5f * (-width * s - height * c)

            return centerY + doubleArrayOf(p1, p2, p3, p4).min()!!
        }
    val top: Double
        get() {
            val c: Double = cos(angle)
            val s: Double = sin(angle)

            val p1: Double = .5f * (-width * s + height * c)
            val p2: Double = .5f * (width * s + height * c)
            val p3: Double = .5f * (width * s - height * c)
            val p4: Double = .5f * (-width * s - height * c)

            return centerY + doubleArrayOf(p1, p2, p3, p4).max()!!
        }

    fun translate(dx: Double, dy: Double): Boolean {
        val c: Double = cos(-angle)
        val s: Double = sin(-angle)

        centerX -= dx * c - dy * s
        centerY -= dx * s + dy * c

        onViewPortChangedListener?.onViewPortMoved()

        return t()
    }

    fun scale(focusX: Double, focusY: Double, scaleFactor: Double): Boolean {
        centerX = (centerX - focusX) * scaleFactor + focusX
        centerY = (centerY - focusY) * scaleFactor + focusY

        width /= scaleFactor
        height /= scaleFactor

        onViewPortChangedListener?.onViewPortMoved()

        val s: Boolean = s()
        val t: Boolean = t()
        return s || t
    }

    fun rotate(focusX: Double, focusY: Double, angle: Double) {
        this.angle += angle

        val c: Double = cos(-angle)
        val s: Double = sin(-angle)

        val cx: Double = centerX - focusX
        val cy: Double = centerY - focusY

        val nx: Double = cx * c - cy * s
        val ny: Double = cx * s + cy * c

        centerX = nx + focusX
        centerY = ny + focusY

        onViewPortChangedListener?.onViewPortAngleChanged(this.angle)

        s()
        t()
    }

    fun setAngle(angle: Double) {
        this.angle = angle
        onViewPortChangedListener?.onViewPortAngleChanged(angle)
        s()
        t()
    }

    fun setOnViewPortChangedListener(callback: OnViewPortChangedListener) {
        onViewPortChangedListener = callback
    }

    fun t(): Boolean {
        var v = false
        var h = false

        val left: Double = left
        val right: Double = right
        if (left < -1f) {
            centerX = -1f + .5f * (right - left)
            h = true
        } else if (right > 1f) {
            centerX = 1f - .5f * (right - left)
            h = true
        }

        val top: Double = top
        val bottom: Double = bottom
        if (top > 1f) {
            centerY = 1f - .5f * (top - bottom)
            v = true
        } else if (bottom < -1f) {
            centerY = -1f + .5f * (top - bottom)
            v = true
        }

        return v && h
    }

    fun s(): Boolean {
        val w: Double = abs(right - left)
        if (w > maxWidth) {
            val k: Double = maxWidth / w
            width *= k
            height *= k
            return true
        }

        if (width < minWidth) {
            val k: Double = minWidth / width
            width *= k
            height *= k
            return true
        }

        val h: Double = abs(top - bottom)
        if (h > maxWidth) {
            val k: Double = maxWidth / h
            width *= k
            height *= k
            return true
        }

        return false
    }
}