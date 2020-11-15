import java.awt.Point;
import java.awt.Color;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ThreadLocalRandom;
import java.util.function.Function;
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
                String.format("\"color\": %d", getArgb(color)) +
                "}";
        sendCustomData(data);
    }

    public void polyline(List<Point> points, Color color) {
        String polyline = getJsonString(points, p -> Stream.of(p.getX(), p.getY()));
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.POLYLINE.getName()) +
                String.format("\"points\": %s,", polyline) +
                String.format("\"color\": %d", getArgb(color)) +
            "}";
        sendCustomData(data);
    }

    public void circle(Point p, double radius, Color color, boolean fill) {
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.CIRCLE.getName()) +
                String.format("\"p\": [%f, %f],", p.getX(), p.getY()) +
                String.format("\"r\": %f,", radius) +
                String.format("\"color\": %d,", getArgb(color)) +
                String.format("\"fill\": %b", fill) +
            "}";
        sendCustomData(data);
    }

    public void rectangle(Point tl, Point br, boolean fill, Color... color) {
        String colors = getJsonString(Arrays.asList(color), c -> Stream.of(getArgb(c)));
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.RECTANGLE.getName()) +
                String.format("\"tl\": [%f, %f],", tl.getX(), tl.getY()) +
                String.format("\"br\": [%f, %f],", br.getX(), br.getY()) +
                String.format("\"color\": %s,", colors) +
                String.format("\"fill\": %b", fill) +
            "}";
        sendCustomData(data);
    }

    public void triangle(Point p1, Point p2, Point p3, boolean fill, Color... color) {
        String polyline = getJsonString(Arrays.asList(p1, p2, p3), p -> Stream.of(p.getX(), p.getY()));
        String colors = getJsonString(Arrays.asList(color), c -> Stream.of(getArgb(c)));
        String data =
            "{" +
                String.format("\"type\": \"%s\",", PrimitiveType.TRIANGLE.getName()) +
                String.format("\"points\": %s,", polyline) +
                String.format("\"color\": %s,", colors) +
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
                String.format("\"message\": \"%s\"", message) +
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

    private <T, E extends Stream<?>> String getJsonString(List<T> points, Function<T, E> mapper) {
        if (points.size() == 1) {
            return points.stream()
                .flatMap(mapper)
                .findFirst().orElseThrow().toString();
        }
        return points.stream()
            .flatMap(mapper)
            .map(Object::toString)
            .collect(Collectors.joining(",", "[", "]"));
    }

    private int getArgb(Color color) {
        return (color.getAlpha() << 24) | color.getRGB();
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
            rc.rectangle(getRandomPoint(), getRandomPoint(), true, Color.BLACK);
            rc.circle(getRandomPoint(), 3, Color.GREEN, true);
            rc.line(getRandomPoint(),  getRandomPoint(), Color.YELLOW);
            rc.polyline(
                Arrays.asList(getRandomPoint(),  getRandomPoint(), getRandomPoint()),
                Color.RED
            );
            rc.triangle(getRandomPoint(),  getRandomPoint(), getRandomPoint(), true, Color.BLUE, Color.BLACK, Color.GREEN);
            rc.circlePopup(getRandomPoint(), 200, "bla bla");
            rc.rectPopup(getRandomPoint(),  getRandomPoint(), "bla bla 2");
            rc.message("Hello World" + i);

            rc.endFrame();
        }
    }

    private static Point getRandomPoint() {
        return new Point(
            ThreadLocalRandom.current().nextInt(1, 400),
            ThreadLocalRandom.current().nextInt(1, 400)
        );
    }
}