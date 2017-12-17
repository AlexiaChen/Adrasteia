import React from 'react';
import PropTypes from 'prop-types';
import ListItem from '../ListItem';

const propTypes = {
    items: PropTypes.array.isRequired,
    onSelect: PropTypes.func.isRequired
};

function List(props){
    
    const itemsContent = props.items.map(
        item => 
         <ListItem item={item} key={item.id} onClick={() => props.onSelect(item.id)}/>
    );
    
    return (
        <div className="list-component col-md-4 list-group">
            {itemsContent}
        </div>
    );
}

List.propTypes = propTypes;

export default List;