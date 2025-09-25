const WebSocket = require("ws");
const os = require("os");

const PORT = process.env.PORT || 8080;
const wss = new WebSocket.Server({ port: PORT });

// Local IP ထုတ်ဖို့ function
function getLocalIp() {
  const nets = os.networkInterfaces();
  for (const name of Object.keys(nets)) {
    for (const net of nets[name]) {
      if (net.family === "IPv4" && !net.internal) {
        return net.address;
      }
    }
  }
  return "127.0.0.1";
}

console.log(`✅ WebSocket server running at ws://${getLocalIp()}:${PORT}`);

wss.on("connection", (ws) => {
  console.log("Client connected");

  ws.on("message", (msg) => {
    console.log("Received:", msg.toString());

    // 👉 Relay message to all connected clients
    wss.clients.forEach((client) => {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(msg.toString());
      }
    });
  });

  ws.on("close", () => console.log("Client disconnected"));
});
