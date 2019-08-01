package jaakko.leinonen.com.mapview

fun map(input: Float, imin: Float, imax: Float, omin: Float, omax: Float): Float = omin + (input - imin) / (imax - imin) * (omax - omin)
fun map(x: Int, imin: Int, imax: Int, omin: Int, omax: Int): Int = (omin.toFloat() + (x.toFloat() - imin.toFloat()) / (imax.toFloat() - imin.toFloat()) * (omax.toFloat() - omin.toFloat())).toInt()
fun map(x: Double, imin: Double, imax: Double, omin: Double, omax: Double): Double = omin + (x - imin) / (imax - imin) * (omax - omin)
