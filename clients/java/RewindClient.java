import java.awt.Point;
import java.awt.Color;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class RewindClient {
    private static final String DEFAULT_HOST = "127.0.0.1";
    private static final int DEFAULT_PORT = 9111;

    private final Socket socket;
    private final OutputStream outputStream;

    enum PrimitiveType {
        CIRCLE("circle"),
        RECTANGLE("rectangle"),
        TRIANGLE("triangle"),
        POLYLINE("polyline"),
        MESSAGE("message"),
        POPUP("popup"),
        OPTIONS("options"),
        END("end");
        private final String name;

        PrimitiveType(String name) {
            this.name = name;
        }

        public String getName() {
            return name;
        }
    }

    public static RewindClient createClient(String host, int port) {
        return new RewindClient(host, port);
    }

    public static RewindClient createClient() {
        return new RewindClient(DEFAULT_HOST, DEFAULT_PORT);
    }

    public void line(Point p1, Point p2, Color color) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.POLYLINE.getName()) +
                String.format("\"points\": [%f, %f, %f, %f],", p1.getX(), p1.getY(), p2.getX(), p2.getY()) +
                String.format("\"color\": %d", color.getRGB()) +
                "}";
        sendCustomData(data);
    }

    public void polyline(List<Point> points, Color color) {
        String polyline = getPointsString(points);
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.POLYLINE.getName()) +
                String.format("\"points\": [%s],", polyline) +
                String.format("\"color\": %d", color.getRGB()) +
            "}";
        sendCustomData(data);
    }

    public void circle(Point p, double radius, Color color, boolean fill) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.CIRCLE.getName()) +
                String.format("\"p\": [%f, %f],", p.getX(), p.getY()) +
                String.format("\"r\": %f,", radius) +
                String.format("\"color\": %d,", color.getRGB()) +
                String.format("\"fill\": %b", fill) +
            "}";
        sendCustomData(data);
    }

    public void rectangle(Point tl, Point br, Color color, boolean fill) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.RECTANGLE.getName()) +
                String.format("\"tl\": [%f, %f],", tl.getX(), tl.getY()) +
                String.format("\"br\": [%f, %f],", br.getX(), br.getY()) +
                String.format("\"color\": %d,", color.getRGB()) +
                String.format("\"fill\": %b", fill) +
            "}";
        sendCustomData(data);
    }

    public void triangle(Point p1, Point p2, Point p3, Color color, boolean fill) {
        String polyline = getPointsString(Arrays.asList(p1, p2, p3));
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.TRIANGLE.getName()) +
                String.format("\"points\": [%s],", polyline) +
                String.format("\"color\": %d,", color.getRGB()) +
                String.format("\"fill\": %b", fill) +
            "}";
        sendCustomData(data);
    }

    public void circlePopup(Point p, double radius, String message) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.POPUP.getName()) +
                String.format("\"p\": [%f, %f],", p.getX(), p.getY()) +
                String.format("\"r\": %f,", radius) +
                String.format("\"text\": \"%s\"", message) +
            "}";
        sendCustomData(data);
    }

    public void rectPopup(Point tl, Point br, String message) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.POPUP.getName()) +
                String.format("\"tl\": [%f, %f],", tl.getX(), tl.getY()) +
                String.format("\"br\": [%f, %f],", br.getX(), br.getY()) +
                String.format("\"text\": \"%s\"", message) +
            "}";
        sendCustomData(data);
    }

    public void message(String message) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.MESSAGE.getName()) +
                String.format("\"text\": \"%s\"", message) +
            "}";
        sendCustomData(data);
    }

    public void options(int layer, boolean permanent) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.OPTIONS.getName()) +
                String.format("\"layer\": %d,", layer) +
                String.format("\"permanent\": %b", permanent) +
            "}";
        sendCustomData(data);
    }

    public void endFrame() {
        String data =
            "{" +
                String.format("\"type\": \"%s\"", PrimitiveType.END.getName()) +
            "}";
        sendCustomData(data);
    }

    public void close() {
        try {
            socket.close();
        } catch (IOException e) {
            throw new RuntimeException("Can't close socket");
        }
    }

    private void sendCustomData(String data) {
        try {
            outputStream.write(data.getBytes());
            outputStream.flush();
        } catch (IOException e) {
            throw new RuntimeException("Can't send data");
        }
    }

    private String getPointsString(List<Point> points) {
        return points.stream()
            .flatMap(point -> Stream.of(point.getX(), point.getY()))
            .map(d -> String.format("%f", d))
            .collect(Collectors.joining(","));
    }

    private RewindClient(String DEFAULT_HOST, int DEFAULT_PORT) {
        try {
            socket = new Socket(DEFAULT_HOST, DEFAULT_PORT);
            socket.setTcpNoDelay(true);
            outputStream = socket.getOutputStream();
        } catch (IOException e) {
            throw new RuntimeException("Can't open connection");
        }
    }

    public static void main(String[] args) {
        RewindClient rc = RewindClient.createClient();
        rc.options(1, true);
        for (int i = 0; i < 100; i++) {
            rc.rectangle(getRandomPoint(), getRandomPoint(), Color.BLACK, true);
            rc.circle(getRandomPoint(), 3, Color.GREEN, true);
            rc.line(getRandomPoint(),  getRandomPoint(), Color.YELLOW);
            rc.polyline(
                Arrays.asList(getRandomPoint(),  getRandomPoint(), getRandomPoint()),
                Color.RED
            );
            rc.triangle(getRandomPoint(),  getRandomPoint(), getRandomPoint(), Color.BLUE, true);
            rc.circlePopup(getRandomPoint(), 200, "bla bla");
            rc.rectPopup(getRandomPoint(),  getRandomPoint(), "bla bla 2");
            rc.message("Hello World");

            rc.endFrame();
        }
    }

    private static Point getRandomPoint() {
        ThreadLocalRandom.current().nextInt(1, 400);

        return new Point(
            ThreadLocalRandom.current().nextInt(1, 400),
            ThreadLocalRandom.current().nextInt(1, 400)
        );
    }
}