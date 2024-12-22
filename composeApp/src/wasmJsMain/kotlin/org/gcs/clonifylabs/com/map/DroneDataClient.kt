package org.gcs.clonifylabs.com.map

import kotlinx.atomicfu.TraceBase
import kotlinx.browser.document
import kotlinx.browser.window
import org.w3c.dom.WebSocket
import org.w3c.dom.events.Event
import org.w3c.dom.MessageEvent

class DroneDataClient {
    private var socket: WebSocket? = null
    private var isConnected = false
    public var telemetryData: TelemetryData = TelemetryData(GpsData(),ImuData(),HeartbeatData(),RcChannelsData(),BatteryData())
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
}

