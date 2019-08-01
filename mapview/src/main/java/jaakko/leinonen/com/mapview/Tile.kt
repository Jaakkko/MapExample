package jaakko.leinonen.com.mapview

data class Tile(val layer: Int, val col: Int, val row: Int, val x: Float, val y: Float, val tileWidth: Float, val cos: Float, val sin: Float)