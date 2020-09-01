var path = require('path');
const HtmlWebpackPlugin = require("html-webpack-plugin"); 
const InlineSourceWebpackPlugin = require('inline-source-webpack-plugin'); 
const UglifyJsPlugin = require('uglifyjs-webpack-plugin'); 

module.exports = { 
 
    // Path to your entry point. From this file Webpack will begin his work 
    entry: './wled00/data/index.js', 
   
    // Path and filename of your result bundle. 
    // Webpack will bundle all JavaScript into this file 
    output: { 
      path: path.resolve(__dirname, 'wled00/data/dist'), 
      filename: 'index.js'
    }, 
   
    mode: 'development', 
   
    module: { 
      rules: [ 
        { 
          test: /\.js$/, 
          exclude: /(node_modules)/, 
          use: { 
            loader: 'babel-loader', 
            options: { 
              presets: ['@babel/preset-env'] 
            } 
          } 
        } 
      ] 
    },
    optimization: { 
      minimize: true, 
      minimizer: [new UglifyJsPlugin({ 
        sourceMap: false, 
      })] 
    }, 
    plugins: [ 
      new HtmlWebpackPlugin({ 
        inlineSource: '.(js|css)$', // embed all javascript and css inline, 
        template: path.resolve(__dirname, "wled00/data", "index.htm"), 
        hash: false 
      }), 
      new InlineSourceWebpackPlugin({ 
        compress: true, 
        rootpath: './wled00/data/dist', 
        noAssetMatch: 'warn' 
      }) 
    ]
};
