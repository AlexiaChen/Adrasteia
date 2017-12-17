import React from 'react';
import PropTypes from 'prop-types';
import './style.less';

const propTypes = {
    onClick: PropTypes.func.isRequired
};

function CreateBar(props) {
    return (
        <a href="#" onClick={props.onClick} className="list-group-item create-bar-component"> 
         + Create New Blog
        </a>
    );
}

CreateBar.propTypes = propTypes;

export default CreateBar;