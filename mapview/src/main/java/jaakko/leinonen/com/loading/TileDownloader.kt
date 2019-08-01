package jaakko.leinonen.com.loading

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import java.io.InputStream
import java.net.HttpURLConnection
import java.net.URL

class TileDownloader(val urls: Array<LoadURL>) {
	companion object {
		val options: BitmapFactory.Options = BitmapFactory.Options().apply {
			inPreferredConfig = Bitmap.Config.ARGB_8888
		}
	}

	interface LoadURL {
		val map: String
		val compressFormat: Bitmap.CompressFormat
		val alpha: Float

		fun tileExists(layer: Int, row: Int, col: Int): Boolean

		fun getUrl(layer: Int, row: Int, col: Int): String
	}

	fun downloadTile(loadIndex: Int, layer: Int, row: Int, col: Int): Bitmap? {
		var connection: HttpURLConnection? = null
		var inputStream: InputStream? = null
		return try {
			connection = URL(urls[loadIndex].getUrl(layer, row, col)).openConnection() as HttpURLConnection
			connection.doInput = true
			connection.connect()
			if (connection.responseCode != 200)
				return null

			inputStream = connection.inputStream
			BitmapFactory.decodeStream(inputStream, null, options)
		} catch (e: Exception) {
			e.printStackTrace()
			null
		} finally {
			inputStream?.close()
			connection?.disconnect()
		}
	}
}


