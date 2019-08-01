package jaakko.leinonen.com.caching

import android.content.Context
import android.content.SharedPreferences
import androidx.room.Room
import java.io.File
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicLong

/**
 * LRU Cache. Varastoi karttakuvat
 */
class TileCache(private val context: Context) {
    companion object {
        private const val VERSION_FILE = "cache_version"

        const val SHARED_PREFS = "cache_prefs"

        const val KEY_DISK_USAGE = "disk_usage"
        const val KEY_CACHE_SIZE = "cache_size"

        const val DATABASE_NAME = "tile_cache_db"
    }

    private val sharedPreferences: SharedPreferences = context.getSharedPreferences(SHARED_PREFS, Context.MODE_PRIVATE)
    private val diskUsage: AtomicLong = AtomicLong(0)
    private val tileDao: TileDao
    private val cacheSize: Long

    private val cacheExecutor: ExecutorService = Executors.newSingleThreadExecutor()

    init {
        diskUsage.set(sharedPreferences.getLong(KEY_DISK_USAGE, 0))

        cacheSize = sharedPreferences.getLong(KEY_CACHE_SIZE, 128 * 1048576)
        // cacheSize = sharedPreferences.getLong(KEY_CACHE_SIZE, 128 * 1000000)

        val database: TileDatabase = Room.databaseBuilder(context, TileDatabase::class.java, DATABASE_NAME)
            .build()

        tileDao = database.tileDao()

        updateCache()
    }

    fun onTileDownloaded(identifier: String, fileLength: Long) {
        cacheExecutor.execute {
            tileDao.insert(CachedTile(identifier))

            var used: Long = diskUsage.addAndGet(fileLength)
            while (used > cacheSize) {
                tileDao.getLeastRecents().forEach {
                    val file = File(context.filesDir, it.uri)
                    diskUsage.addAndGet(-file.length())
                    file.delete()
                    tileDao.delete(it.id)
                }

                used = diskUsage.get()
            }

            sharedPreferences.edit()
                .putLong(KEY_DISK_USAGE, diskUsage.get())
                .apply()
        }
    }

    fun onTileUsed(identifier: String) {
        cacheExecutor.execute {
            tileDao.delete(identifier)
            tileDao.insert(CachedTile(identifier))
        }
    }

    /**
     * cache_version niminen tiedosto. Sisältää versio numeron.
     * Versio 0: cache_version tiedostoa ei ole.
     * Versio 1: cache_version tiedosto on olemassa. Tiedostot on järjestetty muodossa kartta/layer/row/col
     */
    private fun updateCache() {
        val versionFile = File(context.filesDir, VERSION_FILE)

        // Versio 0
        if (!versionFile.exists()) {
            versionFile.createNewFile()
            versionFile.writeText("1")
        }
    }
}