package org.gcs.clonifylabs.com

import androidx.compose.ui.ExperimentalComposeUiApi
import kotlinx.coroutines.delay
import org.gcs.clonifylabs.com.map.BatteryData
import org.gcs.clonifylabs.com.map.DroneDataClient
import org.gcs.clonifylabs.com.map.DroneDataParser
import org.gcs.clonifylabs.com.map.GpsData
import org.gcs.clonifylabs.com.map.HeartbeatData
import org.gcs.clonifylabs.com.map.ImuData
import org.gcs.clonifylabs.com.map.MapView
import org.gcs.clonifylabs.com.map.RcChannelsData
import org.gcs.clonifylabs.com.map.TelemetryData
import org.gcs.clonifylabs.com.map.WebMapView
import org.w3c.dom.WebSocket

@JsName("setMapLocation")
private external fun setMapLocation(latitude: Double, longitude: Double, zoom: Int)

@JsName("initializeMap")
private external fun initializeMap()

@JsName("addMarker")
private external fun addMarker(lat:Double,lng:Double)

@JsName("moveMarker")
private external fun moveMarker(lat: Double,lng: Double)

@JsName("updateGpsData")
private external fun updateGpsData(latitude:Double, longitude:Double, altitude:Int, fixType:Int, satellitesVisible:Int)

@JsName("updateBatteryData")
private external fun updateBatteryData(voltage:Double, current:Int, remaining:Int)

@OptIn(ExperimentalComposeUiApi::class)
val mapView: MapView = WebMapView()
private var socket: WebSocket? = null
private var isConnected = false
public var telemetryData: TelemetryData = TelemetryData(
    GpsData(),
    ImuData(),
    HeartbeatData(),
    RcChannelsData(),
    BatteryData()
)
fun connect() {
    try {
        socket = WebSocket("ws://localhost:4444")

        socket?.onopen = { event ->
            println("WebSocket bağlantısı açıldı")
            isConnected = true
        }

        socket?.onmessage = { event ->
            val jsonString = event.data.toString()
            try {
                telemetryData = DroneDataParser.parseJson(jsonString)
                println("AAAAAAAAAAA $telemetryData")
                mapView.goToMarker(telemetryData.gps.latitude, telemetryData.gps.longitude)
                updateBatteryData(
                    telemetryData.battery.voltage,
                    telemetryData.battery.current,
                    telemetryData.battery.remaining
                )
                updateGpsData(
                    telemetryData.gps.latitude,
                    telemetryData.gps.longitude,
                    telemetryData.gps.altitude,
                    telemetryData.gps.fix_type,
                    telemetryData.gps.satellites_visible
                )

            } catch (e: Exception) {
                println("Veri işleme hatası: ${e.message}")
            }
        }

        socket?.onclose = { event ->
            println("WebSocket bağlantısı kapandı")
            isConnected = false
        }

        socket?.onerror = { event ->
            println("WebSocket hatası")
        }

    } catch (e: Exception) {
        println("WebSocket bağlantı hatası: ${e.message}")
    }
}

fun disconnect() {
    socket?.close()
    socket = null
    isConnected = false
}

fun isConnected(): Boolean = isConnected


fun main() {

    // Initialize the map
    mapView.initialize()

    // Set the location to New York City
    mapView.setLocation(40.7128, -74.0060, 12)

    mapView.goToMarker(40.711400, -74.008654);

    connect()
    println(telemetryData.gps.latitude)

}
/*fun main() {
    println("Program başladı")
    val client = DroneDataClient()
    client.connect()
}*/
