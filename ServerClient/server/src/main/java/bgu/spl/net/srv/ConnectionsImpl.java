package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import bgu.spl.net.impl.stomp.User;

public class ConnectionsImpl<T> implements Connections<T> {

    Map<Integer,ConnectionHandler<T>> activeClients = new ConcurrentHashMap<>();
    Map<String,User> users = new ConcurrentHashMap<>();
    Map<String,List<Integer>> channelToClient = new ConcurrentHashMap<>(); //map channel to the list of clients 
    Map<Integer,User> clientToUser = new ConcurrentHashMap<>();
    int messageIdCounter = 0;
    
    public boolean send(int connectionId, T msg){
        activeClients.get((Integer)connectionId).send(msg);
        return true;
    }

    public void send(String channel, T msg){}

    public void disconnect(int connectionId){
        //disconnect user
        User user = clientToUser.get((Integer)connectionId);
        if(user!=null){
            Set<String> channels = user.disconnect();
            for(String channel: channels){
                List<Integer> subcribedClients = channelToClient.get(channel);
                subcribedClients.remove((Integer)connectionId);
                if(subcribedClients.isEmpty())
                    channelToClient.remove(channel); //remove channel from map if it has no more subscribed users
            }
            clientToUser.remove((Integer)connectionId);
        }
        //disconnect client
        activeClients.remove((Integer)connectionId);
    }

    //TODO - wasn't part of the skelaton

    public void connectClient(int connectionId, ConnectionHandler<T> ch){
        activeClients.put(connectionId, ch);
    }

    public String connectUser(int connectionId, String username, String password){

        String response = "CONNECTED";
        User currentUser = users.get(username);

        if(currentUser==null){
            User newUser = new User(username, password, connectionId);
            users.put(username, newUser);
            clientToUser.put(connectionId,newUser);
        }
        else{
            if(currentUser.getPassword().equals(password)){
                if(currentUser.isConnected())
                    response = "ALREADY_LOGGEDIN";
                else{
                    currentUser.setConnection(connectionId);
                    clientToUser.put(connectionId,currentUser);
                }
            }
            else response = "WRONG_PASSWORD";
        }
        return response;
    }

    public boolean subscribeToChannel(Integer connectionId, String channel, int subId){

        //add channel to user list
        boolean succeed = clientToUser.get(connectionId).subscribe(channel, subId);
        
        if(succeed){
            //add user to the channel list
            List<Integer> subscribed = channelToClient.get(channel);
            if(subscribed == null){
                subscribed = new CopyOnWriteArrayList<>();
                channelToClient.put(channel, subscribed);
            }
            subscribed.add(connectionId);
        }
        
        return succeed;
    }

    public boolean unsubscribeFromChannel(Integer connectionId, int subId){
        boolean succeed = false;
        String channel = clientToUser.get(connectionId).unsubscribe(subId); //remove channel from user list

        if(channel != null){
            List<Integer> subscribed = channelToClient.get(channel);
            subscribed.remove(connectionId); //remove client from channel list
            if(subscribed.isEmpty())
                channelToClient.remove(channel); //if channel has no subscribers remove channel
            succeed = true;
        }
        return succeed;
    }

    public Map<Integer,Integer> getChannelSubscribers(String channel){
        Map<Integer,Integer> subscribers = new HashMap<Integer,Integer>();
        for(Integer connectionId: channelToClient.get(channel)){
            subscribers.put(connectionId, clientToUser.get(connectionId).getSubId(channel));
        }
        return subscribers;
    }

    public synchronized int generateMessageId(){
        this.messageIdCounter++;
        return this.messageIdCounter;
    }
}
