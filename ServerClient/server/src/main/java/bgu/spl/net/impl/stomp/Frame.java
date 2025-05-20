package bgu.spl.net.impl.stomp;

import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class Frame {

    private String command;
    private Map<String,String> headers;
    private String body;
    private boolean valid = true;
    private String msg;
    
    public Frame(String msg){

        this.msg = msg;

        String[] lines = msg.split("\n");
        this.command = lines[0];
        this.headers = new HashMap<>();
        this.body = "";
        switch(this.command){
            case "CONNECT": buildConnectFrame(lines); break;
            case "DISCONNECT": bulidDisconnectFrame(lines); break;
            case "SUBSCRIBE": bulidSubscribeFrame(lines); break;
            case "UNSUBSCRIBE": bulidUnsubcribeFrame(lines); break;
            case "SEND": buildSendFrame(lines); break;
            default:{
                valid = false;
                body = "ERROR\n"+"message: malformed frame received\n\n"+"The message\n-----\n"+msg+"\n-----\n"+"unknown command: \""+command+"\"\n";
            }
        }
    }

    private void buildHeardesMap(String[] lines, Set<String> keys){
        String missingHeaders = "";
        String unknownHeaders = "";

        int  i = 1;     
        while(/*valid &&*/ i < lines.length && !lines[i].equals("")){

            String[] pair = lines[i].split(":",2);
            headers.put(pair[0],pair[1]);
            if(! keys.contains(pair[0]))
                unknownHeaders = unknownHeaders+", "+pair[0];
            i++;
 
            // headers.put(pair[0], pair[1]);
            // if(keys.contains(pair[0]))
            //     headers.put(pair[0], pair[1]);
            // else{
            //     //this.valid = false;
            //     unknownHeaders = unknownHeaders+", "+pair[0];
            // }
            //i++; 
        }
        for(String key: keys){
            if((! key.equals("receipt")) && (!headers.containsKey(key))){                 
                missingHeaders = missingHeaders +", "+key;
                valid = false;
            }
        }
        if(!valid){
            String errorFrame = "ERROR\n"+"message:malformed frame received\n";
            String receipId = headers.get("receipt");
            if(receipId!=null)
                errorFrame = errorFrame + "receipt-id:"+receipId+"\n\n";
            errorFrame += "The message\n-----\n"+msg+"\n-----\n";
            if(! missingHeaders.equals("")) 
                errorFrame += "Missing required headers: "+missingHeaders+"\n";
            if(! unknownHeaders.equals(""))
                errorFrame += "Message containing unknown headers: "+unknownHeaders+"\n";

            this.body = errorFrame;
        }
        else{  //build message body
            for(int j = i+1; j < lines.length; j++)
                this.body+=lines[j]+"\n";
            if(!body.equals(""))
                body = body.substring(0, body.length()-1);
        }
    }

    private void buildConnectFrame(String[] lines){
        Set<String> keys = new HashSet<>(Arrays.asList("accept-version","host","login","passcode","receipt"));
        buildHeardesMap(lines,keys);     
    }

    private void bulidDisconnectFrame(String[] lines){
        Set<String> keys = new HashSet<>(Arrays.asList("receipt"));
        buildHeardesMap(lines, keys);
        if(valid && !headers.containsKey("receipt"))
            this.body = "ERROR\n"+"message:malformed frame received\n\n"+"The message\n-----\n"+msg+"\n-----\n"+"missing header: \""+"receipt"+"\"\n";
    }

    private void bulidSubscribeFrame(String[] lines){
        Set<String> keys = new HashSet<>(Arrays.asList("destination","id","receipt"));
        buildHeardesMap(lines, keys);
    }

    private void bulidUnsubcribeFrame(String[] lines){
        Set<String> keys = new HashSet<>(Arrays.asList("id","receipt"));
        buildHeardesMap(lines,keys);     
    }

    private void buildSendFrame(String[] lines){
        Set<String> keys = new HashSet<>(Arrays.asList("destination","receipt"));
        buildHeardesMap(lines, keys);
    }

    public String buildErrorFrame(String error){
        String errorFrame = "ERROR\n"+"message:"+error+"\n";
        String receipId = headers.get("receipt");
        if(receipId!=null)
            errorFrame = errorFrame + "receipt-id:"+receipId+"\n\n";
        errorFrame += "The message\n-----\n"+msg+"\n-----\n";
        return errorFrame;
    }

    public String createMessageFrame(int mgsId, int subId){
        return "MESSAGE\n"+"subscription:"+subId+"\n"+"message-id:"+mgsId+"\n"+"destination:"+headers.get("destination")+"\n\n"+body;
    }

    public String getCommand(){
        return command;
    }
    public Map<String,String> getHeaders(){
        return headers;
    }
    public String getBody(){
        return body;
    }

    public boolean isValid(){
        return this.valid;
    }

    public String createReceipt(){ return "";}
    
}
