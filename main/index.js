    var _base_address = "http://ams-projekt"

    var _updateValues_address = "/api/data"

    var nr_testeres =  10;

    var Update_timer = 2500;

    var testerStatus;

    var sensoriD;

    var Config_values_post;

    var queryElements;

    var checkdiv;

    var w8forbackend  = 0;

    const timeTow8 = 1;


    var HttpClient = function() 
        {
            this.get = function(aUrl, aCallback) 
            {
                var anHttpRequest = new XMLHttpRequest();
                anHttpRequest.onreadystatechange = function() 
                { 
                    if (anHttpRequest.readyState == 4 && anHttpRequest.status == 200)
                    aCallback(anHttpRequest.responseText);
                }
                anHttpRequest.open( "GET", _base_address+aUrl, true );            
                anHttpRequest.send( null );
            }
    }

    var client = new HttpClient();
    

    function update_values() 
    {
            client.get(_updateValues_address, function(response) 
            {
                var ValObj = JSON.parse(response);
                
                if(w8forbackend <= timeTow8)
                {
                    w8forbackend++; 
                }
                console.log(ValObj);

                for (var i = 1; i <= nr_testeres; i++) 
                {
    
                    checkdiv = "sensordiv" + i; 

                        if(document.getElementById(checkdiv) != null)
                        {                        
                            for (var key in ValObj)
                            {

                                    if(ValObj[key]["id"] == i)
                                    {
                                        console.log("here2");
                                        document.getElementById("id" + i).innerHTML =  ValObj[key]["id"];
                                        document.getElementById("temp" + i).innerHTML =  ValObj[key]["temp"];
                                    }
                               
                            }
                       
                        }                                                         
                }
            });   
    } 


Periodic_updater = setInterval(update_values,Update_timer);
