package org.gcs.clonifylabs.com.map

interface MapView {
    fun initialize()
    fun setLocation(latitude: Double, longitude: Double, zoom: Int)
    fun goToMarker(lat:Double,lng:Double)
}