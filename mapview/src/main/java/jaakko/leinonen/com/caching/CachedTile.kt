package jaakko.leinonen.com.caching

import androidx.room.Entity
import androidx.room.PrimaryKey


@Entity
class CachedTile() {
    @PrimaryKey(autoGenerate = true)
    var id: Long = 0

    lateinit var uri: String

    constructor(uri: String) : this() {
        this.uri = uri
    }
}