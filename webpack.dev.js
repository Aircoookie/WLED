var path = require('path');
const { merge } = require('webpack-merge');
const common = require('./webpack.common.js');
const HtmlWebpackPlugin = require("html-webpack-plugin");
const InlineSourceWebpackPlugin = require('inline-source-webpack-plugin');
const TerserPlugin = require('terser-webpack-plugin');

module.exports = merge(common, {
  mode: 'development',
  optimization: {
    minimize: false
  },  
  plugins: [
    new HtmlWebpackPlugin({
      inlineSource: '.(js|css)$', // embed all javascript and css inline,
      template: path.resolve(__dirname, "wled00/data", "index.htm"),
      hash: false,
      minify: false
    }),
    new InlineSourceWebpackPlugin({
      compress: false,
      rootpath: './wled00/data/dist',
      noAssetMatch: 'error'
    })
  ]
});
