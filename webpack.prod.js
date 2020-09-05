var path = require('path');
const { merge } = require('webpack-merge');
const common = require('./webpack.common.js');
const HtmlWebpackPlugin = require("html-webpack-plugin");
const InlineSourceWebpackPlugin = require('inline-source-webpack-plugin');
const TerserPlugin = require('terser-webpack-plugin');

module.exports = merge(common, {
  mode: 'production',
  optimization: {
    minimize: true,
    minimizer: [new TerserPlugin()] // used to minify JavaScript
  },  
  plugins: [
    new HtmlWebpackPlugin({
      inlineSource: '.(js|css)$', // embed all javascript and css inline,
      template: path.resolve(__dirname, "wled00/data", "index.htm"),
      hash: false,
      minify: {
        // see https://github.com/kangax/html-minifier#options-quick-reference
        collapseWhitespace: true,
        conservativeCollapse: true,
        minifyCSS: true,
        removeComments: true,
        removeRedundantAttributes: false, // if true, will remove type="text" from <input> which breaks some css
        sortAttributes: true,
        sortClassName: true
      }
    }),
    new InlineSourceWebpackPlugin({
      compress: true,
      rootpath: './wled00/data/dist',
      noAssetMatch: 'error'
    })
  ]
});
