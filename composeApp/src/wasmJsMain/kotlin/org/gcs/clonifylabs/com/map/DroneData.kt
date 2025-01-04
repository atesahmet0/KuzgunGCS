package org.gcs.clonifylabs.com.map

data class GpsData(
    var latitude: Double = 0.0,
    var longitude: Double = 0.0,
    var altitude: Int = 0,
    var fix_type: Int = 0,
    var satellites_visible: Int = 0
)

data class ImuData(
    var accel_x: Int = 0,
    var accel_y: Int = 0,
    var accel_z: Int = 0,
    var gyro_x: Int = 0,
    var gyro_y: Int = 0,
    var gyro_z: Int = 0,
    var mag_x: Int = 0,
    var mag_y: Int = 0,
    var mag_z: Int = 0
)

data class HeartbeatData(
    var type: Int = 0,
    var autopilot: Int = 0,
    var base_mode: Int = 0,
    var system_status: Int = 0
)

data class RcChannelsData(
    var chan1_raw: Int = 0,
    var chan2_raw: Int = 0,
    var chan3_raw: Int = 0,
    var chan4_raw: Int = 0,
    var chan5_raw: Int = 0,
    var chan6_raw: Int = 0,
    var chan7_raw: Int = 0,
    var chan8_raw: Int = 0
)

data class BatteryData(
    var voltage: Double = 0.0,
    var current: Int = 0,
    var remaining: Int = 0
)
data class MissionItem(
    var seq: Int = 0,
    var frame: Int = 0,
    var command: Int = 0,
    var current: Int = 0,
    var autocontinue: Int = 0,
    var param1: Double = 0.0,
    var param2: Double = 0.0,
    var param3: Double = 0.0,
    var param4: Double = Double.NaN,
    var x: Double = 0.0,
    var y: Double = 0.0,
    var z: Double = 0.0,
    var mission_type: Int = 0
)

data class CurrentMissionData(
    var mission_items: ArrayList<MissionItem> = ArrayList()
)


data class TelemetryData(
    val gps: GpsData = GpsData(),
    val imu: ImuData = ImuData(),
    val heartbeat: HeartbeatData = HeartbeatData(),
    val rcChannels: RcChannelsData = RcChannelsData(),
    val battery: BatteryData = BatteryData(),
    val current_mission: CurrentMissionData = CurrentMissionData()
)

class DroneDataParser {
    companion object {
        fun parseJson(jsonString: String): TelemetryData {
            var cleanJsonString = jsonString.replace("\\n", "").replace("\n", "")
            cleanJsonString = jsonString.replace(" ", "")
            val gps = GpsData()
            val imu = ImuData()
            val heartbeat = HeartbeatData()
            val rcChannels = RcChannelsData()
            val battery = BatteryData()
            val current_mission = CurrentMissionData()
            try {
                // GPS verilerini çıkar
                val gpsMatch = Regex("\"gps\":\\s*\\{([^}]+)\\}").find(jsonString)
                gpsMatch?.groupValues?.get(1)?.let { gpsContent ->
                    Regex("\"latitude\":\\s*([\\d.-]+)").find(gpsContent)?.groupValues?.get(1)?.let {
                        gps.latitude = it.toDouble()
                    }
                    Regex("\"longitude\":\\s*([\\d.-]+)").find(gpsContent)?.groupValues?.get(1)?.let {
                        gps.longitude = it.toDouble()
                    }
                    Regex("\"altitude\":\\s*(\\d+)").find(gpsContent)?.groupValues?.get(1)?.let {
                        gps.altitude = it.toInt()
                    }
                    Regex("\"fix_type\":\\s*(\\d+)").find(gpsContent)?.groupValues?.get(1)?.let {
                        gps.fix_type = it.toInt()
                    }
                    Regex("\"satellites_visible\":\\s*(\\d+)").find(gpsContent)?.groupValues?.get(1)?.let {
                        gps.satellites_visible = it.toInt()
                    }
                }

                // IMU verilerini çıkar
                val imuMatch = Regex("\"imu\":\\s*\\{([^}]+)\\}").find(jsonString)
                imuMatch?.groupValues?.get(1)?.let { imuContent ->
                    Regex("\"accel_x\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.accel_x = it.toInt()
                    }
                    Regex("\"accel_y\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.accel_y = it.toInt()
                    }
                    Regex("\"accel_z\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.accel_z = it.toInt()
                    }
                    Regex("\"gyro_x\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.gyro_x = it.toInt()
                    }
                    Regex("\"gyro_y\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.gyro_y = it.toInt()
                    }
                    Regex("\"gyro_z\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.gyro_z = it.toInt()
                    }
                    Regex("\"mag_x\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.mag_x = it.toInt()
                    }
                    Regex("\"mag_y\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.mag_y = it.toInt()
                    }
                    Regex("\"mag_z\":\\s*([\\d.-]+)").find(imuContent)?.groupValues?.get(1)?.let {
                        imu.mag_z = it.toInt()
                    }
                }

                // Heartbeat verilerini çıkar
                val heartbeatMatch = Regex("\"heartbeat\":\\s*\\{([^}]+)\\}").find(jsonString)
                heartbeatMatch?.groupValues?.get(1)?.let { heartbeatContent ->
                    Regex("\"type\":\\s*(\\d+)").find(heartbeatContent)?.groupValues?.get(1)?.let {
                        heartbeat.type = it.toInt()
                    }
                    Regex("\"autopilot\":\\s*(\\d+)").find(heartbeatContent)?.groupValues?.get(1)?.let {
                        heartbeat.autopilot = it.toInt()
                    }
                    Regex("\"base_mode\":\\s*(\\d+)").find(heartbeatContent)?.groupValues?.get(1)?.let {
                        heartbeat.base_mode = it.toInt()
                    }
                    Regex("\"system_status\":\\s*(\\d+)").find(heartbeatContent)?.groupValues?.get(1)?.let {
                        heartbeat.system_status = it.toInt()
                    }
                }

                // RC Kanal verilerini çıkar
                val rcMatch = Regex("\"rc_channels\":\\s*\\{([^}]+)\\}").find(jsonString)
                rcMatch?.groupValues?.get(1)?.let { rcContent ->
                    Regex("\"chan1_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan1_raw = it.toInt()
                    }
                    Regex("\"chan2_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan2_raw = it.toInt()
                    }
                    Regex("\"chan3_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan3_raw = it.toInt()
                    }
                    Regex("\"chan4_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan4_raw = it.toInt()
                    }
                    Regex("\"chan5_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan5_raw = it.toInt()
                    }
                    Regex("\"chan6_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan6_raw = it.toInt()
                    }
                    Regex("\"chan7_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan7_raw = it.toInt()
                    }
                    Regex("\"chan8_raw\":\\s*(\\d+)").find(rcContent)?.groupValues?.get(1)?.let {
                        rcChannels.chan8_raw = it.toInt()
                    }
                }

                // Batarya verilerini çıkar
                val batteryMatch = Regex("\"battery\":\\s*\\{([^}]+)\\}").find(jsonString)
                batteryMatch?.groupValues?.get(1)?.let { batteryContent ->
                    Regex("\"voltage\":\\s*([\\d.]+)").find(batteryContent)?.groupValues?.get(1)?.let {
                        battery.voltage = it.toDouble()
                    }
                    Regex("\"current\":\\s*([\\d.-]+)").find(batteryContent)?.groupValues?.get(1)?.let {
                        battery.current = it.toInt()
                    }
                    Regex("\"remaining\":\\s*(\\d+)").find(batteryContent)?.groupValues?.get(1)?.let {
                        battery.remaining = it.toInt()
                    }
                }
                // Current Mission verilerini çıkar
                val currentMissionRegex = "\"current_mission\":\\s*\\{(.*?)\\}(?=\\s*\\})".toRegex(RegexOption.DOT_MATCHES_ALL)
                val currentMissionMatch = currentMissionRegex.find(jsonString)

                currentMissionMatch?.groupValues?.get(1)?.let { currentMissionContent ->

                    // Doğrudan mission items içeriğini al
                    val itemsContent = currentMissionContent.substringAfter("[").substringBefore("]")

                    // Her bir item'ı ayır
                    val items = itemsContent.split("},{").map {
                        it.trim().removeSurrounding("{", "}")
                    }

                    items.forEach { itemContent ->
                        val missionItem = MissionItem()

                        try {
                            // Parse işlemleri
                            Regex("\"seq\":(\\d+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.seq = it.toInt()
                            }
                            Regex("\"frame\":(\\d+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.frame = it.toInt()
                            }
                            Regex("\"command\":(\\d+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.command = it.toInt()
                            }
                            Regex("\"current\":(\\d+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.current = it.toInt()
                            }
                            Regex("\"autocontinue\":(\\d+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.autocontinue = it.toInt()
                            }
                            Regex("\"param1\":([\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.param1 = it.toDouble()
                            }
                            Regex("\"param2\":([\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.param2 = it.toDouble()
                            }
                            Regex("\"param3\":([\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.param3 = it.toDouble()
                            }
                            Regex("\"param4\":(nan|[\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.param4 = if (it == "nan") Double.NaN else it.toDouble()
                            }
                            Regex("\"x\":([\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.x = it.toDouble()
                            }
                            Regex("\"y\":([\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.y = it.toDouble()
                            }
                            Regex("\"z\":([\\d.-]+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.z = it.toDouble()
                            }
                            Regex("\"mission_type\":(\\d+)").find(itemContent)?.groupValues?.get(1)?.let {
                                missionItem.mission_type = it.toInt()
                            }

                            current_mission.mission_items.add(missionItem)

                        } catch (e: Exception) {
                            println("Error parsing item: ${e.message}")
                        }
                    }
                }
            } catch (e: Exception) {
                println("Parse hatası: ${e.message}")
            }

            return TelemetryData(gps, imu, heartbeat, rcChannels, battery,current_mission)
        }
    }
}