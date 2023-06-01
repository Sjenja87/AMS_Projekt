    var _base_address = "http://ams-projekt"

    var _updateValues_address = "/api/data"

    var nr_testeres =  8;

    var Update_timer = 1000;

    var testerStatus;

    var sensoriD;

    var Config_values_post;

    var queryElements;

    var checkdiv;

    var w8forbackend  = 0;

    const timeTow8 = 1;

    var States = ["Active","Inactive"];


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

                    for(key in ValObj) {

                        
                            if(document.getElementById(checkdiv) != null)
                            {                          


                            //         var newDiv = document.createElement('div');
                            //         newDiv.id = "testerdiv" + i;
                            //         newDiv.className = 'container';
                                    
                            //         dublicantDiv = document.getElementById('sensordiv1');
                            //         newDiv.innerHTML = dublicantDiv.innerHTML;
                    
                            //         document.getElementById("wrapper").appendChild(newDiv);
                    
                            //         queryElements = document.querySelectorAll("[id='ID1']");
                    
                            //         queryElements[1].id = "ID" + i;
                                
                            //         queryElements = document.querySelectorAll("[id='Status1']");
                    
                            //         queryElements[1].id = "Status" + i;
                    
                            //         document.getElementById(("SensorName" + i)).innerHTML = "Sensor " + i;

                            //         document.getElementById(checkdiv).style.display = "block";
                        
                            //     if(document.getElementById(checkdiv).style.display === "" || document.getElementById(checkdiv).style.display === "none")
                            //     {
                            //         document.getElementById(checkdiv).style.display = "block";
                            //     }
                                
                            // }
                       
                            // if(document.getElementById(checkdiv) != null)
                            // {
                                
                            //     document.getElementById(checkdiv).style.display = "none";                          
                                
                         }      
                    }                                                   
                }
            });   
    } 


Periodic_updater = setInterval(update_values,Update_timer);
