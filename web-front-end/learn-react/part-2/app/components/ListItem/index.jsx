import React from 'react';
import PropTypes from 'prop-types';
import './style.less';

const propTypes = {
    item: PropTypes.object.isRequired,
    onClick: PropTypes.func.isRequired
};


function ListItem(props){
    let formatTime = 'unknown time';
    if(!props.item.time){
        formatTime = new Date().toISOString();
    }else{
        formatTime = props.item.time;
    }
    return (
        <a href="#" className="list-group-item item-component" onClick={props.onClick}> 
            <span className="item-title">{props.item.title}</span>
            <span className="label label-default label-pill pull-xs-right item-time">{formatTime}</span>
        </a>
    );
}

ListItem.propTypes = propTypes;

export default ListItem;