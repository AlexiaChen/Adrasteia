const path = require('path');
var htmlWebpackPlugin = require('html-webpack-plugin'); // npm install html-webpack-plugin --save-dev

module.exports = {
    entry: {  //4个页面入口
        main: './src/script/main.js',
        a : './src/script/a.js',
        b: './src/script/b.js',
        c: './src/script/c.js'
    },
    output: {
        path: path.resolve(__dirname, 'dist'), // 必须是绝对路径
        filename: "js/[name]-bundle.js", // [name]是个内置变量，此外还有[hash] [chunkhask] hash为本次打包的hash
        // chunkhash可以认为是文件“版本号”，也就是文件的内容hash
        publicPath: 'http://cdn.fire.com' //会把这个地址在引用资源的地址上加入前缀url，上线地址
    },

    plugins: [ //根据index模板生成多个页面
        new htmlWebpackPlugin({
            template: 'index.html', //context是当前的根目录，所以不需要指定路径
            filename: 'index-[hash].html',
            inject: 'body', // 把引入的脚本放入到html body标签中
            title: 'webpack is awesome!',
            date: new Date(),
            minify: {  // 压缩，这样线上文件就降低流量带宽了
                removeComments:true, // 删除注释
                collapseWhitespace:true //删除空格
            }
        }),
        new htmlWebpackPlugin({
            template: 'index2.html', 
            filename: 'a.html',
            inject: 'head', 
            title: 'a html title',
            date: new Date(),
            chunks:['main','a']
        }),
        new htmlWebpackPlugin({
            template: 'index2.html', 
            filename: 'b.html',
            inject: 'head', 
            title: 'b html title',
            date: new Date(),
            chunks: ['b']
        }),
        new htmlWebpackPlugin({
            template: 'index2.html', 
            filename: 'c.html',
            inject: false, 
            title: 'c html title',
            date: new Date(),
            chunks: ['c']
        })
    ]
}