import  './layer.less';
import tpl from './layer.html';
import ejs_tpl from './layer.ejs';

function layer(){
    return {
        name: 'layer',
        tpl: tpl,
        ejs: ejs_tpl
    };
}

export default layer;