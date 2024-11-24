package org.gcs.clonifylabs.com

import androidx.compose.ui.ExperimentalComposeUiApi
import org.gcs.clonifylabs.com.map.MapView
import org.gcs.clonifylabs.com.map.WebMapView

@JsName("setMapLocation")
private external fun setMapLocation(latitude: Double, longitude: Double, zoom: Int)

@JsName("initializeMap")
private external fun initializeMap()

@JsName("addMarker")
private external fun addMarker(lat:Double,lng:Double)

@JsName("moveMarker")
private external fun moveMarker(lat: Double,lng: Double)

@OptIn(ExperimentalComposeUiApi::class)
fun main() {
    val mapView: MapView = WebMapView()

    // Initialize the map
    mapView.initialize()

    // Set the location to New York City
    mapView.setLocation(40.7128, -74.0060, 12)

    mapView.goToMarker(40.711400, -74.008654);

}
