package org.gcs.clonifylabs.com

import androidx.compose.ui.ExperimentalComposeUiApi
import kotlinx.browser.window
import kotlinx.coroutines.delay
import kotlinx.coroutines.internal.ThreadSafeHeap
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
import org.jetbrains.compose.resources.getResourceUri
import org.w3c.dom.WebSocket
import org.w3c.dom.events.Event
import org.w3c.dom.events.MouseEvent
import kotlin.js.JsName

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

@JsName("getCoordinates")
private external fun getCoordinates() : String

@JsName("isCanFly")
private external fun isCanFly(): Boolean

@JsName("resetFly")
private external fun resetFly()

@JsName("getUpload")
private external fun getUpload() : Boolean

@JsName("resetUpload")
private external fun resetUpload()

@JsName("getAutoRTL")
private external fun getAutoRTL():Boolean

@JsName("resetAutoRTL")
private external fun resetAutoRTL()

@JsName("getDeleteMission")
private external fun getDeleteMission():Boolean

@JsName("resetDeleteMission")
private external fun resetDeleteMission()

@JsName("getRTL")
private external fun getRTL() : Boolean

@JsName("resetRTL")
private external fun resetRTL()

@JsName("getMission")
private external fun getMission() :Boolean

@JsName("uploadWaypointData")
private external fun uploadWaypointData(lat: Double,lng: Double,altitude: Double,command : Int)

@JsName("resetMission")
private external fun resetMission()

@JsName("getRefresh")
private external fun getRefresh() : Boolean

@JsName("resetRefresh")
private external fun resetRefresh()

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
fun convertCoordinates(coordinatesJson: String): List<String> {
    val coordinates = mutableListOf<String>()
    val pattern = """"lat":([\d.]+),"lng":([\d.]+)""".toRegex()
    val matches = pattern.findAll(coordinatesJson)

    matches.forEach { match ->
        val lat = match.groupValues[1]
        val lng = match.groupValues[2]
        coordinates.add("waypoint,$lat,$lng")
    }

    return coordinates
}

fun connect() {
    var enableFly = true
    var isFirstGetMission = true
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
                if (getMission()){
                    if(isFirstGetMission){
                        isFirstGetMission = false;
                        telemetryData.current_mission.mission_items.forEach { waypoint ->
                            uploadWaypointData(waypoint.x,waypoint.y,waypoint.z,waypoint.command)
                        }
                    }else{
                        socket?.send("getMission")
                        window.setTimeout("",2000)
                        telemetryData.current_mission.mission_items.forEach { waypoint ->
                            uploadWaypointData(waypoint.x,waypoint.y,waypoint.z,waypoint.command)
                        }
                    }

                    resetMission()
                }

            } catch (e: Exception) {
                println("Veri işleme hatası: ${e.message}")
            }
            var coordinateList = convertCoordinates(getCoordinates())
            if(coordinateList.size > 0 && isCanFly()) {

                socket?.send("fly")
                println("ok")
                resetFly()
            }
            if (getUpload()){
                coordinateList.forEach { i ->
                    socket?.send(i)
                }
                socket?.send("uploadMission")
                resetUpload()
            }
            if (getAutoRTL()){
                socket?.send("autoRTL")
                resetAutoRTL()
            }
            if (getDeleteMission()){
                socket?.send("deleteMissions")
                resetDeleteMission()
            }
            if (getRTL()){
                socket?.send("RTL")
                resetRTL()
            }
            if (getRefresh()){
                coordinateList = mutableListOf()
                //socket?.send("getMission")
                resetRefresh()
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
    mapView.setLocation(47.352780, 8.342743, 12)

    mapView.goToMarker(47.352780, 8.342743);

    connect()



}
/*fun main() {
    println("Program başladı")
    val client = DroneDataClient()
    client.connect()
}*/
