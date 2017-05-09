const path = require('path');
var htmlWebpackPlugin = require('html-webpack-plugin'); // npm install html-webpack-plugin --save-dev

module.exports = {
    entry: './src/app.js',
    output: {
        path: path.resolve(__dirname, 'dist'), // 必须是绝对路径
        filename: "js/[name]-bundle.js" 
    },

    plugins: [
        new htmlWebpackPlugin({
            filename: 'index.html',
            template: 'index.html',
            inject: 'body'
        })
    ],

    module: {
        loaders: [
            {
                test: /.js$/,
                exclude: path.resolve(__dirname, 'node_modules/'),
                include: path.resolve(__dirname, 'src/'),
                loader: 'babel-loader',
                query: {
                    presets: [
                        'latest'
                    ]
                }   // 可以删除，然后在package.json中指定也可以
            }
        ]
    }
}