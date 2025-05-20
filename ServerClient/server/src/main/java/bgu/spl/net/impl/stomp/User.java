package bgu.spl.net.impl.stomp;

import java.nio.channels.Channel;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class User {
    
    private String username;
    private String password;
    private int connectionId = -1;
    private Map<Integer,String> idToChannel;
    private Map<String,Integer> channelToId;
    
    public User(String _username, String _password){
        username = _username;
        password = _password;
        idToChannel = new ConcurrentHashMap<>();
        channelToId = new ConcurrentHashMap<>();
    }

    public User(String _username, String _password, int _connectionId){
        this(_username, _password);
        connectionId = _connectionId;
    }

    public String getPassword(){
        return password;
    }

    public boolean isConnected(){
        return connectionId!=-1;
    }

    /* add channel to the list of channels the user is subscribed to */
    public boolean subscribe(String channel, Integer subId){
        boolean succeed = false;
        if(channelToId.putIfAbsent(channel, subId) == null){
            succeed = true;
            idToChannel.put(subId, channel);
        }
        return succeed;
    }

    //remove the channel with subscribed subId and return which channel it 
    public String unsubscribe(Integer subId){
        String channel = idToChannel.remove(subId); 
        if(channel != null)
            channelToId.remove(channel);
        return channel;
    }

    public void setConnection(int connectionId){
        this.connectionId = connectionId;
    }

    /* disconect user from client and remove all subscriptions */
    /* returns a set of the channels the user was subscribed to */
    public Set<String> disconnect(){
        Set<String> channels = new HashSet<>();
        for (String channel: channelToId.keySet())
            channels.add(channel);
        channelToId.clear();
        idToChannel.clear();
        this.connectionId = -1;
        return channels;
    }

    public int getSubId(String channel){
        return channelToId.get(channel);
    }

    public boolean isSubscribed(String channel){
        return channelToId.get(channel)!=null;
    }
}
