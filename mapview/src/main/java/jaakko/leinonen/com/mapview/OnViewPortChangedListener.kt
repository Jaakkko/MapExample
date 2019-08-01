package jaakko.leinonen.com.mapview


interface OnViewPortChangedListener {
    fun onViewPortAngleChanged(angle: Double)
    fun onViewPortMoved()
}
