package jaakko.leinonen.com.caching

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.Query

@Dao
interface TileDao {

    @Query("DELETE FROM CachedTile WHERE uri = :uri")
    fun delete(uri: String)

    @Query("DELETE FROM CachedTile WHERE id = :id")
    fun delete(id: Long)

    @Insert
    fun insert(tile: CachedTile)

    @Query("SELECT * FROM CachedTile ORDER BY id ASC LIMIT 5")
    fun getLeastRecents(): Array<CachedTile>

    @Query("DELETE FROM CachedTile")
    fun clear()

}