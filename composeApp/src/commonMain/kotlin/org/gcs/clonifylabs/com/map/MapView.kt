package org.gcs.clonifylabs.com.map

interface MapView {
    fun initialize()
    fun setLocation(latitude: Double, longitude: Double, zoom: Int)
}