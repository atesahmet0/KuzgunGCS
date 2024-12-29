import websockets
import asyncio

async def send_fly_command():
    async with websockets.connect('ws://localhost:4444') as websocket:
        await websocket.send("waypoint,42.43543,8.53243")
        await websocket.send("waypoint,43.43543,8.53243")
        await websocket.send("waypoint,44.43543,8.53243")
        await websocket.send("waypoint,45.43543,8.53243")
        await websocket.send('fly')
        print("Sent 'fly' command")

asyncio.get_event_loop().run_until_complete(send_fly_command())
"""
// Mevcut konuma göre göreceli mission noktaları
/*
    {47.35278 , 8.342743, 50},    // Başlangıç noktası (mevcut konum)
    {47.398500, 8.547000, 50},    // Yaklaşık 100m kuzeydoğu
    {47.397500, 8.546500, 50},    // Yaklaşık 100m güneydoğu
    {47.398200, 8.545500, 50},    // Yaklaşık 100m batı
    {47.398000, 8.546160, 50}     // Başlangıç noktasına dönüş
"""