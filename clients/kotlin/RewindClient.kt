import java.awt.Color
import java.io.OutputStream
import java.net.Socket
import java.util.concurrent.ThreadLocalRandom
import kotlin.random.Random

class RewindClient(val host: String = "127.0.0.1", val port: Int = 9111) {
    private val socket: Socket = Socket(host, port)
    private val outputStream: OutputStream = socket.getOutputStream()

    init {
        socket.tcpNoDelay = true
    }

    fun line(p1: Pair<Double, Double>, p2: Pair<Double, Double>, color: Color) {
        val type = PrimitiveType.POLYLINE
        val polyline = getJsonArray(listOf(p1, p2)) { p -> listOf(p.first, p.second) }
        val data = """{"type": "${type.value}", "points": $polyline, "color": ${getArgb(color)}}"""
        sendCustomData(data)
    }

    fun polyline(points: List<Pair<Double, Double>>, color: Color) {
        val type = PrimitiveType.POLYLINE
        val polyline = getJsonArray(points) { p -> listOf(p.first, p.second) }
        val data = """{"type": "${type.value}", "points": $polyline, "color": ${getArgb(color)}}"""
        sendCustomData(data)
    }

    fun circle(p: Pair<Double, Double>, radius: Double, color: Color, fill: Boolean) {
        val type = PrimitiveType.CIRCLE
        val data = """{"type": "${type.value}", "p": [${p.first}, ${p.second}], "radius": $radius, "color": "${getArgb(color)}", "fill": $fill}"""
        sendCustomData(data)
    }

    fun rectangle(tl: Pair<Double, Double>, br: Pair<Double, Double>, fill: Boolean, vararg color: Color) {
        val type = PrimitiveType.RECTANGLE
        val colors = getJsonArray(color.asList()) { c -> listOf(getArgb(c)) }
        val data = """{"type": "${type.value}", "tl": [${tl.first}, ${tl.second}], "br": [${br.first}, ${br.second}], "color": $colors, "fill": $fill}"""
        sendCustomData(data)
    }

    fun triangle(p1: Pair<Double, Double>, p2: Pair<Double, Double>, p3: Pair<Double, Double>, fill: Boolean, vararg color: Color) {
        val points = getJsonArray(listOf(p1, p2, p3)) { p -> listOf(p.first, p.second) }
        val colors = getJsonArray(color.asList()) { c -> listOf(getArgb(c)) }
        val type = PrimitiveType.TRIANGLE
        val data = """{"type": "${type.value}", "points": $points, "color": $colors, "fill": $fill}"""
        sendCustomData(data)
    }

    fun circlePopup(p: Pair<Double, Double>, radius: Double, message: String?) {
        val type = PrimitiveType.POPUP
        val data = """{"type": "${type.value}", "p": [${p.first}, ${p.second}], "r": $radius, "text": "$message"}"""
        sendCustomData(data)
    }

    fun rectPopup(tl: Pair<Double, Double>, br: Pair<Double, Double>, message: String?) {
        val type = PrimitiveType.POPUP
        val data = """{"type": "${type.value}", "tl": [${tl.first}, ${tl.second}], "br": [${br.first}, ${br.second}], "text": "$message"}"""
        sendCustomData(data)
    }

    fun options(layer: Int?, permanent: Boolean?) {
        val type = PrimitiveType.OPTIONS
        val data = """{"type": "${type.value}", "layer": $layer, "permanent": $permanent}"""
        sendCustomData(data)
    }

    fun endFrame() {
        val type = PrimitiveType.END
        val data = """{"type": "${type.value}"}"""
        sendCustomData(data)
    }

    fun message(message: String) {
        val type = PrimitiveType.MESSAGE
        val data = """{"type": "${type.value}", "message": "$message"}"""
        sendCustomData(data)
    }

    fun close() {
        socket.close()
    }

    private fun sendCustomData(data: String) {
        outputStream.write(data.toByteArray())
        outputStream.flush()
    }

    private fun <T, E> getJsonArray(elements: List<T>, mapper: (T) -> List<E>): String {
        return if (elements.size == 1) {
            elements
                    .flatMap(mapper)
                    .first()
                    .toString()
        } else {
            elements
                    .flatMap(mapper)
                    .joinToString(separator = ",", prefix = "[", postfix = "]")
        }
    }

    private fun getArgb(color: Color): Int {
        return color.alpha shl 24 or color.rgb
    }

    enum class PrimitiveType(val value: String) {
        CIRCLE("circle"),
        RECTANGLE("rectangle"),
        TRIANGLE("triangle"),
        POLYLINE("polyline"),
        MESSAGE("message"),
        POPUP("popup"),
        OPTIONS("options"),
        END("end");
    }
}

fun main() {
    val rc = RewindClient()
    rc.options(1, true)

    for (i in 0..99) {
        val transparentBlack = Color(0, 0, 0, 70)
        rc.rectangle(getRandomPoint(), getRandomPoint(), true, transparentBlack)
        rc.circle(getRandomPoint(), 334.0, Color.GREEN, true)
        rc.line(getRandomPoint(), getRandomPoint(), Color.YELLOW)
        rc.polyline(
                listOf(getRandomPoint(), getRandomPoint(), getRandomPoint()),
                Color.RED
        )
        rc.triangle(getRandomPoint(), getRandomPoint(), getRandomPoint(), true, Color.BLUE, Color.RED, Color.MAGENTA)
        rc.circlePopup(getRandomPoint(), 200.0, "bla bla")
        rc.rectPopup(getRandomPoint(), getRandomPoint(), "bla bla 2")
        rc.message("Hello World$i")
        rc.endFrame()
    }
    rc.close()
}

fun getRandomPoint(): Pair<Double, Double> {
    return Pair(
            Random.nextDouble(1.0, 400.0),
            ThreadLocalRandom.current().nextDouble(1.0, 400.0)
    )
}
