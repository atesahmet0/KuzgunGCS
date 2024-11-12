package org.gcs.clonifylabs.com

interface Platform {
    val name: String
}

expect fun getPlatform(): Platform