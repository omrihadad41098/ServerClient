package bgu.spl.net.impl.stomp;

import java.util.Map;

import javax.lang.model.util.ElementScanner6;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private volatile boolean terminate = false;
    private Connections<String> connections = null;
    int connectionId;
    
    /**
	 * Used to initiate the current client protocol with it's personal connection ID and the connections implementation
	**/
    public void start(int connectionId, Connections<String> connections){
        this.connections = connections;
        this.connectionId = connectionId;
    }
    
    
    public void process(String message){

        Frame frame = new Frame(message);
        if(!frame.isValid())
            connections.send(connectionId,frame.getBody());
        else{       
            String command = frame.getCommand();
            switch(command){
                case "CONNECT": processConnectFrame(frame); break;
                case "DISCONNECT": processDisconnectFrame(frame); break;
                case "SUBSCRIBE": processSubscribeFrame(frame); break;
                case "UNSUBSCRIBE": processUnsubsribeFrame(frame); break;
                case "SEND": processSendFrame(frame); break;
            }
        }        
    }
	
	/**
     * @return true if the connection should be terminated
     */
    public boolean shouldTerminate(){
        return this.terminate;
    }

    public Connections<String> getConnections(){
        return this.connections;
    } 

    public void processSendFrame(Frame frame){

        Map<String,String> headers = frame.getHeaders();
        String channel = headers.get("destination");
        int msgId = connections.generateMessageId();

        Map<Integer,Integer> channelSubs = connections.getChannelSubscribers(channel);
        if(channelSubs.containsKey((Integer)this.connectionId)){
            for(Integer cId: channelSubs.keySet())
                connections.send(cId, frame.createMessageFrame(msgId, channelSubs.get(cId)));
            
            //send receipt if needed
            String receiptID = headers.get("receipt");
            if(receiptID != null)
                sendReceipt(receiptID);
        }
        else
            sendErrorFrame(frame, "User is not subscribed to channel: "+channel);
    }

    public void processConnectFrame(Frame frame){

        Map<String,String> headers = frame.getHeaders();
        String response = connections.connectUser(connectionId,headers.get("login"),headers.get("passcode"));

        if(response.equals("ALREADY_LOGGEDIN"))
            sendErrorFrame(frame, "User already logged in");
        else{
            if(response.equals("WRONG_PASSWORD"))
                sendErrorFrame(frame, "Wrong password");
            else{
                String connectedFrame = "CONNECTED\n"+"version:"+headers.get("accept-version")+"\n"+"\n\n";
                connections.send(connectionId,connectedFrame);

                //send receipt if needed
                String receiptID = headers.get("receipt");
                if(receiptID != null)
                    sendReceipt(receiptID);
            }
        }
    }

    public void processDisconnectFrame(Frame frame){
        sendReceipt(frame.getHeaders().get("receipt")); //send receipt
        connections.disconnect(connectionId);  //disconnect user from client and client from server
        this.terminate = true;
    }

    public void processSubscribeFrame(Frame frame){

        Map<String,String> headers = frame.getHeaders();

        Integer subId = Integer.parseInt(headers.get("id"));
        String channel = headers.get("destination");
        if(! connections.subscribeToChannel(connectionId,channel,subId))
            sendErrorFrame(frame,"user is already subscribed to this channel");
        else{
            //send receipt if needed
            String receiptID = headers.get("receipt");
            if(receiptID != null)
                sendReceipt(receiptID);
        }
    }

    public void processUnsubsribeFrame(Frame frame){

        Map<String,String> headers = frame.getHeaders();
        Integer subId = Integer.parseInt(headers.get("id"));
        if(! connections.unsubscribeFromChannel(connectionId, subId))
            sendErrorFrame(frame, "user doesn't have a subscription under "+subId+" Id");
        else{
            //send receipt if needed
            String receiptID = headers.get("receipt");
            if(receiptID != null)
                sendReceipt(receiptID);
        }
    }

    private void sendReceipt(String receiptId){
        connections.send(connectionId,"RECEIPT\nreceipt-id:"+receiptId+"\n\n");
    }

    private void sendErrorFrame(Frame frame, String error){
        connections.send(connectionId,frame.buildErrorFrame(error));
        connections.disconnect(this.connectionId);
        this.terminate = true;
    }
    
}
