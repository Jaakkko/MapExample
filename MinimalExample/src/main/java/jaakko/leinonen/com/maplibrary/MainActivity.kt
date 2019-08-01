package jaakko.leinonen.com.maplibrary

import android.graphics.Bitmap
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import jaakko.leinonen.com.loading.TileDownloader
import jaakko.leinonen.com.mapview.MapView
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        mapView.mapUrls = arrayOf(object : TileDownloader.LoadURL {
        	override val map: String = "landscape"
        	override val compressFormat: Bitmap.CompressFormat = Bitmap.CompressFormat.PNG
        	override val alpha: Float = 1.0f

        	override fun tileExists(layer: Int, row: Int, col: Int): Boolean = true

        	override fun getUrl(layer: Int, row: Int, col: Int): String {
                val apiKey: String = <Insert your api key here>
        		return "https://tile.thunderforest.com/$map/$layer/$col/$row.png?apikey=$apiKey"
        	}
        })
    }
}
