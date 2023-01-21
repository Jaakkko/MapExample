Map engine written from scratch.
<img src="https://github.com/Jaakkko/MapLibrary/blob/master/kartta.gif?raw=true">

# Features
- LRU cache
- multiple layers at the same time
- fast downloading times
- smooth touch gestures
- rotation

# Simple example
`MainActivity.kt`
```kotlin
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
                val apiKey: String = "<Insert your api key here>"
                return "https://tile.thunderforest.com/$map/$layer/$col/$row.png?apikey=$apiKey"
            }
        })
    }
}
```
`activity_main.xml`
```xml
<?xml version="1.0" encoding="utf-8"?>
<jaakko.leinonen.com.mapview.MapView
    android:id="@+id/mapView"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    xmlns:android="http://schemas.android.com/apk/res/android" />
```
