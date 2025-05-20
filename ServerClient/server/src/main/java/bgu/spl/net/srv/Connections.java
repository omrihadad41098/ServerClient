package bgu.spl.net.srv;

//import java.io.IOException;
import java.util.Map;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    //TODO - possible more functions

    /**
     * adds a new client to the stractur and generate a new client Id
     * @param ch - the new client connection handler
     * @return - client's unique id
     */
    String connectUser(int connectionId, String username, String password);

    void connectClient(int connectionId, ConnectionHandler<T> ch);

    boolean subscribeToChannel(Integer connectionId, String channel, int subId);

    boolean unsubscribeFromChannel(Integer connectionId, int subId);

    Map<Integer, Integer> getChannelSubscribers(String channel);

    int generateMessageId();
    
}
