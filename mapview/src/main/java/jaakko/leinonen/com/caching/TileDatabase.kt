package jaakko.leinonen.com.caching

import androidx.room.Database
import androidx.room.RoomDatabase

@Database(entities = [CachedTile::class], version = 1)
abstract class TileDatabase : RoomDatabase() {
    abstract fun tileDao(): TileDao
}