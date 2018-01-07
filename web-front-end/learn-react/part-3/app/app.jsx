import React from 'react';
import ReactDom from 'react-dom';
import ToDo from './components/ToDo';

const app  = document.createElement('div');
document.body.appendChild(app);

ReactDom.render(<ToDo />,app);