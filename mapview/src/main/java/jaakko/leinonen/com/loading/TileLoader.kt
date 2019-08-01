package jaakko.leinonen.com.loading

import android.content.Context
import android.content.res.Resources
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.DisplayMetrics
import jaakko.leinonen.com.caching.TileCache
import java.io.File
import java.io.FileOutputStream
import java.util.*
import java.util.concurrent.*

class TileLoader(private val context: Context, private val urls: Array<TileDownloader.LoadURL>) {
    companion object {
        private const val KEEP_ALIVE_TIME = 1L
    }

    private val mTileDownloader: TileDownloader = TileDownloader(urls)

    private val tileCache: TileCache = TileCache(context)

    val loadLock = Any()
    val loaded: Queue<Load> = LinkedList()

    private val numberOfCores: Int = Runtime.getRuntime().availableProcessors()
    private val screenArea: Int by lazy {
        val dm: DisplayMetrics = Resources.getSystem().displayMetrics
        val w: Int = dm.widthPixels
        val h: Int = dm.heightPixels
        (w / 256) * (h / 256)
    }
    private val mQueue: BlockingQueue<Runnable> = LinkedBlockingDeque()
    private val executor: ExecutorService = ThreadPoolExecutor(numberOfCores, screenArea,
        KEEP_ALIVE_TIME, TimeUnit.MINUTES, mQueue)

    fun runInBackground(f: () -> Unit) {
        executor.execute(f)
    }
    fun add(load: Load) {
        synchronized(loadLock) {
            loaded.add(load)
        }
    }
    fun load(loadIndex: Int, tileIndex: Long, layer: Int, row: Int, col: Int): Load {
        val loadUrl: TileDownloader.LoadURL = mTileDownloader.urls[loadIndex]
        val identifier = "${loadUrl.map}/$layer/$row/$col"
        return if (loadUrl.tileExists(layer, row, col)) (getTile(identifier) ?: mTileDownloader.downloadTile(loadIndex, layer, row, col).apply {
            if (this != null) {
                val fileLength: Long = saveTile(loadUrl.compressFormat, identifier, this)
                tileCache.onTileDownloaded(identifier, fileLength)
            }
        }).run {
            tileCache.onTileUsed(identifier)

            Load(tileIndex, this, true, loadUrl.alpha)
        }
        else Load(tileIndex, null, false, 0.0f)
    }

    private fun getTile(identifier: String): Bitmap? {
        return try {
            val file = File(context.filesDir, identifier)
            if (file.exists()) BitmapFactory.decodeFile(file.path, TileDownloader.options)
            else null
        } catch (e: Exception) {
            null
        }
    }

    private fun saveTile(compressFormat: Bitmap.CompressFormat, identifier: String, bitmap: Bitmap): Long {
        val file = File(context.filesDir, identifier)
        file.parentFile.mkdirs()
        FileOutputStream(file).use {
            bitmap.compress(compressFormat, 100, it)
        }

        return file.length()
    }


    class Load(val index: Long, val bitmap: Bitmap?, val haveTile: Boolean, val alpha: Float)
}