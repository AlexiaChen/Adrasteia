import React from 'react';
import PropTypes from 'prop-types';

const propTypes = {
    onClick: PropTypes.func.isRequired
};

function CreateButton({onClick}){
    return (
        <div className="create-btn-component">
            <button onClick={ () => { onClick(); } }>创建新的ToDo</button>
        </div>
    );
}

CreateButton.propTypes = propTypes;

export default CreateButton;