package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        // TODO: implement this

        int port = Integer.parseUnsignedInt(args[0]);
        String serverType = args[1];
        
        if(serverType.equals("tpc")) {
            Server.threadPerClient(
                    port,
                    () -> new StompMessagingProtocolImpl(), //protocol factory
                    () -> new StompMessageEncoderDecoder() //message encoder decoder factory
            ).serve();
        }
        else{
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port,
                    () -> new StompMessagingProtocolImpl(), //protocol factory
                    () -> new StompMessageEncoderDecoder() //message encoder decoder factory
            ).serve();
        }
    }
}
