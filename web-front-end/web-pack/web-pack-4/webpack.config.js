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
                test: /\.js$/,
                exclude: path.resolve(__dirname, 'node_modules/'),
                include: path.resolve(__dirname, 'src/'),
                loader: 'babel-loader',
                query: {
                    presets: [
                        'latest'
                    ]
                }   
            },
            {
                test: /\.css$/,
                loader: 'style-loader!css-loader?importLoaders=1',
                
            },
            {
                test: /\.css$/,
                loader:'postcss-loader',
                options: {
                    plugins: (loader) => [
                    require('autoprefixer')({
                        browsers: ['last 5 versions']
                    })
                    ]
                }

            },
            {
                test: /\.less$/,
                loader: "style-loader!css-loader!postcss-loader!less-loader"
            },
            {
                test: /\.scss$/,
                loader: "style-loader!css-loader!postcss-loader!sass-loader"

            },
            {
                test: /\.html$/,
                loader: "html-loader"
            },
            {
                test: /\.ejs$/,
                loader: "ejs-loader"
            },
            {
                test: /\.(png|jpg|gif|svg)$/i,
                loaders: ['url-loader?limit=2000&name=assets/[name]-[hash:5].[ext]', //如果超过 2KB的图片，那么就采用urlloader对图片进行Base64编码
                'image-webpack-loader'] // 压缩图片
               
            }
           /* {
                test: /\.(png|jpg|gif|svg)$/i,
                loader: "file-loader",
                query: {
                    name: "assets/[name]-[hash:5].[ext]"
                }
            }*/
        ],
       
    }
}