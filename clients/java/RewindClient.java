import java.awt.Color;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;


/**
 * Java client for Rewind viewer. Put this file to the same default package where Strategy/MyStrategy/Runner and other
 * files are extracted.
 * <p>
 * Sample usage:
 * <pre>
 * {@code
 *
 *  private final RewindClient rewindClient = new RewindClient();
 *
 *  @Override
 *  public void move(Wizard self, World world, Game game, Move move) {
 *      initializeTick(self, world, game, move);
 *      for (Wizard w : world.getWizards()) {
 *          RewindClient.Side side = w.getFaction() == self.getFaction() ? RewindClient.Side.OUR : RewindClient.Side.ENEMY;
 *          rewindClient.livingUnit(w.getId(), w.getX(), w.getY(), w.getRadius(), w.getLife(), w.getMaxLife(), side);
 *      }
 *      ...
 *      rewindClient.endFrame();
 *  }
 * }
 * </pre>
 */
public class RewindClient {

    private final Socket socket;
    private final OutputStream outputStream;

    public enum Side {
        OUR(-1),
        NEUTRAL(0),
        ENEMY(1);
        final int side;

        Side(int side) {
            this.side = side;
        }
    }

    /**
     * Should be send on end of move function all turn primitives can be rendered after that point
     */
    void endFrame() {
        send("{\"type\":\"end\"}");
    }

    void circle(double x, double y, double r, Color color) {
        send(String.format("{\"type\": \"circle\", \"x\": %f, \"y\": %f, \"r\": %f, \"color\": %d}", x, y, r, color.getRGB()));
    }

    void rect(double x1, double y1, double x2, double y2, Color color) {
        send(String.format("{\"type\": \"rectangle\", \"x1\": %f, \"y1\": %f, \"x2\": %f, \"y2\": %f, \"color\": %d}", x1, y1, x2, y2, color.getRGB()));
    }

    void line(double x1, double y1, double x2, double y2, Color color) {
        send(String.format("{\"type\": \"line\", \"x1\": %f, \"y1\": %f, \"x2\": %f, \"y2\": %f, \"color\": %d}", x1, y1, x2, y2, color.getRGB()));
    }

    void livingUnit(double x, double y, double r, int hp, int maxHp,
                    Side side) {
        livingUnit(x, y, r, hp, maxHp, side, 0, 0);
    }

    void close() {
        try {
            outputStream.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Pass arbitrary user message to be stored in frame
     * Message content displayed in separate window inside viewer
     * Can be used several times per frame
     *
     * @param msg .
     */
    public void message(String msg) {
        String s = "{\"type\": \"message\", \"message\" : \"" + msg + " \")";
        send(s);
    }

    /**
     * @param x        - x pos of the unit
     * @param y        - y pos of the unit
     * @param r        - radius of the unit
     * @param hp       - current health
     * @param maxHp    - max possible health
     * @param side     - owner of the unit
     * @param course   rotation of the unit - angle in radians [0, 2 * pi) counter clockwise
     * @param unitType - id of unit type (see UnitType enum: https://github.com/kswaldemar/rewind-viewer/blob/master/src/viewer/Frame.h)
     *                 to set texture
     */
    void livingUnit(double x, double y, double r, int hp, int maxHp,
                    Side side, double course, int unitType) {
        send(String.format(
                "{\"type\": \"unit\", \"x\": %f, \"y\": %f, \"r\": %f, \"hp\": %d, \"max_hp\": %d, \"enemy\": %d, \"unit_type\":%d, \"course\": %.3f}",
                x, y, r, hp, maxHp, side.side, unitType, course));
    }

    public RewindClient(String host, int port) {
        try {
            socket = new Socket(host, port);
            socket.setTcpNoDelay(true);
            outputStream = socket.getOutputStream();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public RewindClient() {
        this("127.0.0.1", 7000);
    }

    private void send(String buf) {
        try {
            outputStream.write(buf.getBytes());
            outputStream.flush();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
