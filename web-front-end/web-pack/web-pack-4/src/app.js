import './css/common.css';
import Layer from './components/layer/layer.js';

function App(){
    
    var dom = document.getElementById("app");
    var ejs_dom = document.getElementById("ejs");
    var layer = new Layer();
    dom.innerHTML = layer.tpl;
    ejs_dom.innerHTML = layer.ejs({
        name: "MathxH Chen",
        arr: ["apple","pie","xiao bin"]
    });

    const NUM = 1;
    alert(NUM);
    console.log(layer);
}

new App();