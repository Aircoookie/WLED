var path = require('path');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const HtmlWebpackPlugin = require("html-webpack-plugin");
const InlineSourceWebpackPlugin = require('inline-source-webpack-plugin');
const TerserPlugin = require('terser-webpack-plugin');

module.exports = {

  // Path to your entry point. From this file Webpack will begin his work
  entry: './wled00/data/index.js',

  // Path and filename of your result bundle.
  // Webpack will bundle all JavaScript into this file
  output: {
    path: path.resolve(__dirname, 'wled00/data/dist'),
    filename: 'index.js'
  },
  module: {
    rules: [
      {
        test: /\.m?js$/,
        exclude: /(node_modules|bower_components)/,
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
    minimizer: [new TerserPlugin()] // used to minify JavaScript
  }
};
