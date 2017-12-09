var path = require('path');
var webpack = require('webpack');
var HtmlwebpackPlugin = require('html-webpack-plugin');

var ROOT_PATH = path.resolve(__dirname);
var BUILD_PATH = path.resolve(ROOT_PATH, 'build');
var APP_PATH = path.resolve(ROOT_PATH, 'app');

module.exports= {
  entry: {
    app: path.resolve(APP_PATH, 'app.jsx')
  },
  
  output: {
    path: BUILD_PATH,
    filename: '[name].js'
  },

  //enable dev source map
  devtool: 'eval-source-map',
  //enable dev server
  devServer: {
    historyApiFallback: true,
    hot: true,
    inline: true,
    progress: true
  },
  
  resolve: {
    extensions: ['.js', '.jsx'],
    alias: {
      js: './app'
    }
  },
  
  module: {
   
  rules: [
    {
       test: /\.jsx$/,
       include: APP_PATH,
       use: ['eslint-loader', 'babel-loader']
    },
    {
      test: /\.css$/,
      use: ['style-loader', 'css-loader']
    }
  ]
    
  },
  plugins: [
    new HtmlwebpackPlugin({
      title: 'My first react app Hello'
    })
  ]
}