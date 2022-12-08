package opportunisticcellularnetwork;


 // A source models an entity that forwards packets either from internal or esternal cell
 
simple Source { 
 	parameters:
 	    // testing flag
 	    bool testing = default(false);
 	    // rate of packets inter arrivals 
    	double rate; 	    
 	    // index of module
 	    int id; 	     	    
 	    // max packet dimension
    	int maxPacketSize = 75;
    	// test parameter for packet with fixed size 	
    	int fixedPacketSize = 50;    	
   gates:
        output out;
}
