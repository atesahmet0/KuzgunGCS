package org.gcs.clonifylabs.com.map

@JsName("setMapLocation")
private external fun setMapLocation(latitude: Double, longitude: Double, zoom: Int)

@JsName("initializeMap")
private external fun initializeMap()

@JsName("moveMarker")
private external fun moveMarker(lat: Double,lng: Double)


class WebMapView : MapView {
    override fun initialize() {
        initializeMap()
    }

    override fun setLocation(latitude: Double, longitude: Double, zoom: Int) {
        setMapLocation(latitude, longitude, zoom)
    }
    override fun goToMarker(lat:Double,lng:Double){
        moveMarker(lat,lng)
    }
}