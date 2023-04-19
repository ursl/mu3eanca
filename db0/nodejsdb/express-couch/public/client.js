console.log('Client-side code running');

const button = document.getElementById('myButton');
button.addEventListener('click', function(e) {
    console.log('button was clicked');
    
    fetch('/clicked', {method: 'POST'})
        .then(function(response) {
            console.log("Click was recorded");
            console.log("Click was recorded: response.rescnt ->" + response.get("rescnt") + "<-");
            if(response.ok) {
                console.log("Click was recorded: response ->" + response.rescnt + "<-");
                return;
            }
            throw new Error('Request failed.');
        })
        .catch(function(error) {
            console.log(error);
        });
});

