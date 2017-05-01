const path = require('path');

module.exports = {
    entry: {  //两个页面入口
        main: './src/script/main.js', 
        a: './src/script/a.js'
    },
    output: {
        path: path.resolve(__dirname, 'dist/js'), // 必须是绝对路径
        filename: "[name]-[hash]-bundle.js" // [name]是个内置变量，此外还有[hash] [chunkhask] hash为本次打包的hash
        // chunkhash可以认为是文件“版本号”，也就是文件的内容hash
    }
}