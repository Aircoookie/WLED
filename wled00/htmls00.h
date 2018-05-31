/*
 * Mobile UI by StormPie html
*/
const char PAGE_indexM[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta content='text/html; charset=utf-8' http-equiv='Content-Type'>
  <meta content='width=device-width' name='viewport'>
  <link href="images/safari-pinned-tab.svg" rel="mask-icon">
  <meta content="#ffffff" name="theme-color">
  <meta content="yes" name="apple-mobile-web-app-capable">
  <link href='/favicon.ico' rel='shortcut icon' type='image/x-icon'>
  <title>WLED</title>
  <style>
         *{transition-duration: 0.5s;}
         body {
             margin-bottom: 60px;
             /* Margin bottom by footer height */
             background-color: #333;
             font-family: Helvetica, Verdana, sans-serif;
             color: white;
             background: linear-gradient(0deg, #7e8a96, #333);
             background-size: 400% 400%;
             
         }
         .feedbackanim{
             animation: feedback 0.3s ease;
             animation-iteration-count: 1;
         }
           @-webkit-keyframes feedback {
           0%{background-position:49% 0%}
           50%{background-position:52% 100%}
           100%{background-position:49% 0%}
           }
           @-moz-keyframes feedback {
           0%{background-position:49% 0%}
           50%{background-position:52% 100%}
           100%{background-position:49% 0%}
           }
           @keyframes feedback { 
           0%{background-position:49% 0%}
           50%{background-position:52% 100%}
           100%{background-position:49% 0%}
           }
         
         .segment{
             width:100%; 
             height:145px; 
             max-width:260px; 
             margin-left:auto; 
             margin-right:auto;
         }
         
         .colourCol {
             width: 27px;
             height: 27px;
             float: left;
             border: solid;
             border-radius: 30px;
             padding: 10px;
             margin: 5px;
             font-size: 23px;
         }
         
         #mode li {
             padding: 15px;
             background-color: rgba(255, 255, 255, 0);
             margin: 10px;
             border-color: white;
             color: white;
             border: solid;
             text-transform: uppercase;
             list-style: none;
             margin-left: -37px;
             border-radius: 40px;
         }
         
         #mode li a {
             color: white;
             text-decoration: none;
             font-family: helvetica;
             font-size: 19px;
             display:block;
         }
         
         input[type=range] {
             -webkit-appearance: none;
             width: 228px;
             margin-top: 10px;
             background-color: rgba(0, 0, 0, 0);
         }
         
         input[type=range]:focus {
             outline: none;
         }
         
         input[type=range]::-webkit-slider-runnable-track {
             width: 100%;
             height: 32px;
             cursor: pointer;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
             background: #66d1ff;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#66d1ff', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
             border-radius: 25px;
             border: 0px solid #010101;
         }
         
         input[type=range]::-webkit-slider-thumb {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             height: 26px;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
             -webkit-appearance: none;
             margin-top: 3.3px;
         }
         
         input[type=range]:focus::-webkit-slider-runnable-track {
             background: #66d1ff;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#66d1ff', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
         }
         
         input[type=range]::-moz-range-track {
             width: 100%;
             height: 32px;
             cursor: pointer;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
             background: #66d1ff;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#66d1ff', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
             border-radius: 25px;
             border: 0px solid #010101;
         }
         
         input[type=range]::-moz-range-thumb {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             height: 26px;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
         }
         
         input[type=range]::-ms-track {
             width: 100%;
             height: 32px;
             cursor: pointer;
             background: transparent;
             border-color: transparent;
             color: transparent;
         }
         
         input[type=range]::-ms-fill-lower {
             background: rgba(242, 242, 242, 0.65);
             border: 0px solid #010101;
             border-radius: 50px;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
         }
         
         input[type=range]::-ms-fill-upper {
             background: rgba(255, 255, 255, 0.65);
             border: 0px solid #010101;
             border-radius: 50px;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
         }
         
         input[type=range]::-ms-thumb {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
             height: 26px;
         }
         
         input[type=range]:focus::-ms-fill-lower {
             background: rgba(255, 255, 255, 0.65);
         }
         
         input[type=range]:focus::-ms-fill-upper {
             background: rgba(255, 255, 255, 0.65);
         }
         input[type=range] #speedslider {
             -webkit-appearance: none;
             width: 228px;
             margin-top: 10px;
             background-color: rgba(0, 0, 0, 0);
         }
         
         input[type=range]:focus #speedslider {
             outline: none;
         }
         
         input[type=range]::-webkit-slider-runnable-track #speedslider {
             width: 100%;
             height: 32px;
             cursor: pointer;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
             background: #66d1ff;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#66d1ff', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
             border-radius: 25px;
             border: 0px solid #010101;
         }
         
         input[type=range]::-webkit-slider-thumb #speedslider {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             height: 26px;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
             -webkit-appearance: none;
             margin-top: 3.3px;
         }
         
         input[type=range]:focus::-webkit-slider-runnable-track #speedslider {
             background: #66d1ff;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#66d1ff', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
         }
         
         input[type=range]::-moz-range-track #speedslider {
             width: 100%;
             height: 32px;
             cursor: pointer;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
             background: #66d1ff;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #66d1ff 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#66d1ff', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
             border-radius: 25px;
             border: 0px solid #010101;
         }
         
         input[type=range]::-moz-range-thumb #speedslider {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             height: 26px;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
         }
         
         input[type=range]::-ms-track #speedslider {
             width: 100%;
             height: 32px;
             cursor: pointer;
             background: transparent;
             border-color: transparent;
             color: transparent;
         }
         
         input[type=range]::-ms-fill-lower #speedslider {
             background: rgba(242, 242, 242, 0.65);
             border: 0px solid #010101;
             border-radius: 50px;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
         }
         
         input[type=range]::-ms-fill-upper #speedslider {
             background: rgba(255, 255, 255, 0.65);
             border: 0px solid #010101;
             border-radius: 50px;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
         }
         
         input[type=range]::-ms-thumb #speedslider {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
             height: 26px;
         }
         
         input[type=range]:focus::-ms-fill-lower #speedslider {
             background: rgba(255, 255, 255, 0.65);
         }
         
         input[type=range]:focus::-ms-fill-upper #speedslider {
             background: rgba(255, 255, 255, 0.65);
         }
         input[type=range] #brightslider {
             -webkit-appearance: none;
             width: 228px;
             margin-top: 10px;
             background-color: rgba(0, 0, 0, 0);
         }
         
         input[type=range]:focus #brightslider {
             outline: none;
         }
         
         input[type=range]::-webkit-slider-runnable-track #brightslider {
             width: 100%;
             height: 32px;
             cursor: pointer;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
             background: #111111;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#111111', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
             border-radius: 25px;
             border: 0px solid #010101;
         }
         
         input[type=range]::-webkit-slider-thumb #brightslider {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             height: 26px;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
             -webkit-appearance: none;
             margin-top: 3.3px;
         }
         
         input[type=range]:focus::-webkit-slider-runnable-track #brightslider {
             background: #111111;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#111111', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
         }
         
         input[type=range]::-moz-range-track #brightslider {
             width: 100%;
             height: 32px;
             cursor: pointer;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
             background: #111111;
             /* Old browsers */
             background: -moz-linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* FF3.6-15 */
             background: -webkit-linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* Chrome10-25,Safari5.1-6 */
             background: linear-gradient(45deg, #111111 0%, #ff6a00 100%);
             /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
             filter: progid: DXImageTransform.Microsoft.gradient( startColorstr='#111111', endColorstr='#ff6a00', GradientType=1);
             /* IE6-9 fallback on horizontal gradient */
             border-radius: 25px;
             border: 0px solid #010101;
         }
         
         input[type=range]::-moz-range-thumb #brightslider {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             height: 26px;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
         }
         
         input[type=range]::-ms-track #brightslider {
             width: 100%;
             height: 32px;
             cursor: pointer;
             background: transparent;
             border-color: transparent;
             color: transparent;
         }
         
         input[type=range]::-ms-fill-lower #brightslider {
             background: rgba(242, 242, 242, 0.65);
             border: 0px solid #010101;
             border-radius: 50px;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
         }
         
         input[type=range]::-ms-fill-upper #brightslider {
             background: rgba(255, 255, 255, 0.65);
             border: 0px solid #010101;
             border-radius: 50px;
             box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
         }
         
         input[type=range]::-ms-thumb #brightslider {
             box-shadow: 0.4px 0.4px 0.4px #000031, 0px 0px 0.4px #00004b;
             border: 2.6px solid #ffffff;
             width: 26px;
             border-radius: 26px;
             background: rgba(255, 255, 255, 0);
             cursor: pointer;
             height: 26px;
         }
         
         input[type=range]:focus::-ms-fill-lower #brightslider {
             background: rgba(255, 255, 255, 0.65);
         }
         
         input[type=range]:focus::-ms-fill-upper #brightslider {
             background: rgba(255, 255, 255, 0.65);
         }
         
         .onoffswitch {
             position: relative;
             width: 58px;
             -webkit-user-select: none;
             -moz-user-select: none;
             -ms-user-select: none;
         }
         
         .onoffswitch-checkbox {
             display: none;
         }
         
         .onoffswitch-label {
             display: block;
             overflow: hidden;
             cursor: pointer;
             height: 24px;
             padding: 0;
             line-height: 24px;
             border: 2px solid #999999;
             border-radius: 24px;
             background-color: #EEEEEE;
             transition: background-color 0.3s ease-in;
         }
         
         .onoffswitch-label:before {
             content: "";
             display: block;
             width: 24px;
             margin: 0px;
             background: #FFFFFF;
             position: absolute;
             top: 0;
             bottom: 0;
             right: 32px;
             border: 2px solid #999999;
             border-radius: 24px;
             transition: all 0.3s ease-in 0s;
         }
         
         .onoffswitch-checkbox:checked + .onoffswitch-label {
             background-color: #34A7C1;
         }
         
         .onoffswitch-checkbox:checked + .onoffswitch-label,
         .onoffswitch-checkbox:checked + .onoffswitch-label:before {
             border-color: #34A7C1;
         }
         
         .onoffswitch-checkbox:checked + .onoffswitch-label:before {
             right: 0px;
         }
           section.range-slider {
     position: relative;
     width: 100%;
     height: 50px;
     text-align: center;
  }
  section.range-slider input {
     pointer-events: none;
     position: absolute;
     overflow: hidden;
     left: 0;
     top: 30px;
     width: 100%;
     outline: none;
     height: 32px;
     margin: 0;
     padding: 0;
  }
  section.range-slider input::-webkit-slider-thumb {
     pointer-events: all;
     position: relative;
     z-index: 1;
     outline: 0;
  }
  section.range-slider input::-moz-range-thumb {
     pointer-events: all;
     position: relative;
     z-index: 10;
     -moz-appearance: none;
     width: 9px;
  }
  section.range-slider input::-moz-range-track {
     position: relative;
     z-index: -1;
     background-color: rgba(0, 0, 0, 1);
     border: 0;
  }
  section.range-slider input:last-of-type::-moz-range-track {
     -moz-appearance: none;
     background: none transparent;
     border: 0;
  }
   section.range-slider input[type=range]::-moz-focus-outer {
   border: 0;
  }
  .cyclebtns li{
   width:30%;
   float:left;
   text-align:center;
   border:solid;
   border-color:white;
       margin: 1px;
     padding-top: 10px;
     padding-bottom: 10px;
     border-radius: 29px;
     list-style:none;
     border-radius:30px;
  }
  li {
   box-shadow: rgba(255,255,255,0) 0 0 0px;
  }
  .flash{
   box-shadow: rgba(255,255,255,1) 0 0 80px;
  }
  </style>
</head>
<body style="padding:30px;">
  <!-- Begin page content -->
  <main class="container" role="main" style="max-width:280px; margin-left:auto; margin-right:auto; text-align:center;">
    <div style="width:100%; max-width:260px; margin-left:auto; margin-right:auto; font-size:12px; margin-bottom:20px;">
      <div style="width:50%; float:left; text-align:left;">
        <p>WLED</p><a href="#" onclick="togglePower();"><img src="data:image/png;base64,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" style="height:40px; width:40px; float:left; margin-top:-10px; opacity:0.5;"></a> <a href="/settings"><img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGQAAABkCAYAAABw4pVUAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAHxtJREFUeNrsnQu8VWPex1fXIeWYVEanQiklypRQY7qYmoowKiVe1yaGSDQMpiZCE9NoyhhGvBGmjHLpSBcqpItySSmdLidyihJqUOmi9/99Pvt33vUua++91t77HM28rc9nd077rP2s5/lff//L8+xy+/bt8w5c+89V/gAJDjDkwHWAIf8+V8XgG9999533xRdfePuDb2Eue/fu9Xbv3u1+L1eunFe5cmWvQoUK3qGHHupVrFjxILutsr0q6CP22mX37ty2bds+1vDtt9+6Mfis3e9e5cuXd/9PK612X/Xq1SPdW2oMgRknn3yyZwsqcwZAQIi3a9cu9/shhxzi1apVq1qDBg2OqVOnTpO6des2rlevXv2DDz443whV40c/+lFVuw+mVEoMsddeO40h32zevPlL+/mJXUXr168vtNfKdevWrdm4ceOWL7/80o0PcypVquQIH3bVrFnTe/vtt71q1aqVGQ3KBTXhs88+8+rXr+99/fXXP4hWGBMq//SnPz2pefPmHZo2bdo2Pz+/WV5eXr4Rr5wYpp+pLrTISZwRHQlHw7Zv375l06ZNK1auXDl/yZIls5cuXbpow4YNSSXv8MMP94qKipw2/qAMadSokbd169YymwQS2KpVq9adO3fubs8+64gjjmiC1O7Zs8eZK+YIUSEy7/slOjh/v3kR42AGLzEIreC+r776qriwsHD23LlzJ82ZM+cV06od/rFsHt6qVav+/zDEzFD17t2792rbtu1lpgmnMhfMFUSE+LL3MAWNxdR8/vnnzqxiUnlv586djnHSCjNjXpUqVTzTKu/HP/6xk3L8AETlbzyD+3kxtnySjbt6wYIFE6ZMmTLeNGct45m59FavXv2fzxAb/4gLLrjgytatW/czYtUVUSEMBIIhEN1svpNQfn766afev/71L8cAma2gRvg1hvdFcDQQf3DUUUd5DRs29MwneT/5yU+cpkgLefZBBx2EQHxl5uzpp556avTatWvf//jjj937/5EMqV27dtWLLrromi5dugw0X3Hkjh07nCmRlJrJ8JYtW+Yc6Zo1axxTID6E9aOjqKiHtekF4TUWDII5J510kmf+yjOw4N4HkXEvDLB7d5gwPGY/77E5fcTfYWAur2uvvdY7+uijy54hLKZ3797nGTPuMkk93pyrYwQLh1BowWuvvea9++67ziRxwaSo8DQTOI1WwKCqVat6xx9/vNeuXTvPgIRDdjBG87P5bCkoKBgxbty4MTbX3bmcx+uvv+79/Oc/L1uGNG7cuPb1119/j0nif7FQGIAthxhmGrzp06d7y5cvd0TgfaGjsox1eDaMP+aYY7xOnTp5P/vZzxyjMI8J5OcVFxfPf+CBB26YPXv2olw9e968eV6bNm3KjiG9evU6u1+/fn+1xdX75ptvnMpjemDA888/77333nuOIDCiLIOvZOYNrUFggP3nnHOOI5ZMGRprc99pTn/ImDFjRqLl/zYMMVRSbtCgQcMMxg6WE7Zgzo09efJk79VXX3VoCpPwQzMi7BLSw78Y+HD0wN9BKzRnxYoVk++6666rzOl/vt8zxJxl3uDBgx81e9zDcL4zQWgGDzfk4lnk7JiTLDreXy7ogjABoc8991ynMWg32sJ7Brvfv/vuuy944403lueSITmlijGyzqhRo2accMIJPYCoqDnSNnbsWO8vf/mLt2XLFmeP93dmCDYjOJgwBGnEiBEOesMMzJVpygl33HHHrLPPPrvdfpntNU43GD169EwLpk7FXzBxtGH48OHeSy+95JjD69/tQsMRIgCIMcB766233P8RNPvbEb///e9f7NOnz1n7FUNatmxZ32zqdJOaJlJz4ok777zTW7lypVtArnxFnCx0LjPWrIlMwciRI71p06Y57QGQmG+pOmDAgEnGlG77BUPMTNUeNmzYi+YnjsW+QvyFCxd69957rwvsmHg2BOQ9Fo5E4lgVI0RFTXxGmYBsGYSGMwYm+J///GfJ/238gwzaT+zevXuHnKff41wW4VY12zo5Ly/PaQbMMCfnGV53BIhrooJaJEaAbICiBHDkpiZMmODeT+WLYMavfvUr77DDDvOWLl3qffjhh05AFPVnTLBExoA58AzTDDcXW/8h11133TPmZzrMnz9/WZkzBBX+wx/+8N9HHnnkafgMaYaYkW2aAdjJMy6//HKvSZMmLhcFTGZczCDQmWcmg6116tTxTGIdMw1+O3NDXgpCfvTRR1nND4Ywl0mTJjkfAzRGE00ADx8yZMik/v37n15UVPRZmZqsG2+8cXCzZs3OlwN///33c8YM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" style="height:40px; width:40px; float:left; margin-top:-10px; opacity:0.5;"></a> <a onclick="toggleNight();"><img src="data:image/png;base64,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" style="height:40px; width:40px; float:left; margin-top:-10px; opacity:0.5;"></a>
      </div>
      <div style="width:50%; float:left; text-align:right;">
        <p style="text-align: right; margin-bottom: 5px;">FX 2nd Color</p>
        <div class="onoffswitch" style="float:right;">
          <input class="onoffswitch-checkbox" id="myonoffswitch" name="onoffswitch" type="checkbox"> <label class="onoffswitch-label" for="myonoffswitch"></label>
        </div>
      </div>
    </div>
    <div id="color-picker-container" style="margin-left: auto; margin-right: auto; width:280px;"></div>
    <div>
      <p style="margin-bottom: -4px;">Brightness</p><input id="brightslider" max="255" min="0" type="range" value="222">
    </div>
    <div>
      <p style="margin-bottom: -4px;">Speed</p><input id="speedslider" max="255" min="50" type="range" value="222">
    </div>
    <div class="row segment" style="">
      <div class="colourCol" onclick="presetColour('#ff0000');" style="background-color:#ff0000;">
        &nbsp;
      </div>
      <div class="colourCol" onclick="presetColour('#ff4d00');" style="background-color:#ff4d00;">
        &nbsp;
      </div>
      <div class="colourCol" onclick="presetColour('#ffc800');" style="background-color:#ffc800;">
        &nbsp;
      </div>
      <div class="colourCol" onclick="presetColour('#08ff00');" style="background-color:#08ff00;">
        &nbsp;
      </div><br>
      <div class="colourCol" onclick="presetColour('#0000ff');" style="background-color:#0000ff;">
        &nbsp;
      </div>
      <div class="colourCol" onclick="presetColour('#b700ff');" style="background-color:#b700ff;">
        &nbsp;
      </div>
      <div class="colourCol" onclick="presetColour('#ff00ee');" style="background-color:#ff00ee;">
        &nbsp;
      </div>
      <div class="colourCol" onclick="presetColour('#ffffff');" style="background-color:#ffffff;">
        &nbsp;
      </div>
    </div>
    <div class="row segment" style="height:250px;">
      <div style="height: 51px;">
        <p style="text-align: right; margin-bottom: 5px;">Edit Presets</p>
        <div class="onoffswitch" style="float:right;">
          <input class="onoffswitch-checkbox onoffswitch-checkbox-ps" id="myonoffswitch-ps" name="onoffswitch-ps" type="checkbox"> <label class="onoffswitch-label" for="myonoffswitch-ps"></label>
        </div>
      </div>
      <p style="text-align:center;">Presets</p>
      <div class="colourCol" onclick="preset(1);">
        1
      </div>
      <div class="colourCol" onclick="preset(2);">
        2
      </div>
      <div class="colourCol" onclick="preset(3);">
        3
      </div>
      <div class="colourCol" onclick="preset(4);">
        4
      </div><br>
      <div class="colourCol" onclick="preset(5);">
        5
      </div>
      <div class="colourCol" onclick="preset(6);">
        6
      </div>
      <div class="colourCol" onclick="preset(7);">
        7
      </div>
      <div class="colourCol" onclick="preset(8);">
        8
      </div><br>
      <div class="colourCol" onclick="preset(9);">
        9
      </div>
      <div class="colourCol" onclick="preset(10);">
        10
      </div>
      <div class="colourCol" onclick="preset(11);">
        11
      </div>
      <div class="colourCol" onclick="preset(12);">
        12
      </div><br>
      <div class="colourCol" onclick="preset(13);">
        13
      </div>
      <div class="colourCol" onclick="preset(14);">
        14
      </div>
      <div class="colourCol" onclick="preset(15);">
        15
      </div>
      <div class="colourCol" onclick="preset(16);">
        16
      </div>
    </div>
    <div class="row segment" style="height:50px; padding-top:100px;">
      <p style="text-align: right; margin-bottom: 5px; padding:10px; padding-bottom:0px;">Cycle Mode</p>
      <div class="onoffswitch" style="float:right;">
        <input class="onoffswitch-checkbox onoffswitch-checkbox-cy" id="myonoffswitch-cy" name="onoffswitch-cy" onclick="toggleCycle();" type="checkbox"> <label class="onoffswitch-label" for="myonoffswitch-cy"></label>
      </div>
    </div>
    <p style="padding:15px; padding-bottom:0px;">Cycle Range</p>
    <section class="range-slider">
      <span class="rangeValues" style="font-size:20px;"></span> <input max="16" min="1" step="1" type="range" value="1"> <input max="16" min="1" step="1" type="range" value="2">
    </section>
    <div class="row segment cyclebtns" style="height:240px; padding-top:10px;">
      <ul style="padding:0px;">
        <li onclick="LEDCommand('/win&PC=1&PX=0');">Color</li>
        <li onclick="LEDCommand('/win&PX=1&PC=0');">Effects</li>
        <li onclick="LEDCommand('/win&PX=1&PC=1');">Both</li>
      </ul>
      <div style="padding-top:50px;">
        <p id="PSDurationLabel" style="margin-bottom: -4px;">Preset Duration</p><input id="cyclespeedslider" max="65000" min="50" type="range" step="50" value="5000">
      </div>
      <div>
        <p id="CDurationLabel" style="margin-bottom: -4px;">Color Transition</p><input id="cycletransitionslider" max="65000" min="0" step="50" type="range" value="500">
      </div>
    </div><!--<div class="row" id="gradients" style="margin-top:10px; width:100%; height:145px; max-width:260px; margin-left:auto; margin-right:auto; font-size:24px;">
      </div>-->
    <ul id='mode' style="margin-top:20px;">
      <li>
        <a href="/win&FX=0" target="controlframe">Solid</a>
      </li>
      <li>
        <a href="/win&FX=1" target="controlframe">Blink</a>
      </li>
      <li>
        <a href="/win&FX=2" target="controlframe">Breathe</a>
      </li>
      <li>
        <a href="/win&FX=3" target="controlframe">Wipe</a>
      </li>
      <li>
        <a href="/win&FX=4" target="controlframe">Wipe Random</a>
      </li>
      <li>
        <a href="/win&FX=5" target="controlframe">Random Full Colours</a>
      </li>
      <li>
        <a href="/win&FX=6" target="controlframe">Random Single Colours</a>
      </li>
      <li>
        <a href="/win&FX=7" target="controlframe">Random Multi Colours</a>
      </li>
      <li>
        <a href="/win&FX=8" target="controlframe">Rainbow Full</a>
      </li>
      <li>
        <a href="/win&FX=9" target="controlframe">Rainbow Cycle</a>
      </li>
      <li>
        <a href="/win&FX=10" target="controlframe">Scan</a>
      </li>
      <li>
        <a href="/win&FX=11" target="controlframe">Double Scan</a>
      </li>
      <li>
        <a href="/win&FX=12" target="controlframe">Fade Out</a>
      </li>
      <li>
        <a href="/win&FX=13" target="controlframe">Chase</a>
      </li>
      <li>
        <a href="/win&FX=14" target="controlframe">Chase Rainbow</a>
      </li>
      <li>
        <a href="/win&FX=15" target="controlframe">Running</a>
      </li>
      <li>
        <a href="/win&FX=16" target="controlframe">Twinkle</a>
      </li>
      <li>
        <a href="/win&FX=17" target="controlframe">Twinkle Random</a>
      </li>
      <li>
        <a href="/win&FX=18" target="controlframe">Twinkle Fade</a>
      </li>
      <li>
        <a href="/win&FX=19" target="controlframe">Twinkle Random Fade</a>
      </li>
      <li>
        <a href="/win&FX=20" target="controlframe">Sparkle</a>
      </li>
      <li>
        <a href="/win&FX=21" target="controlframe">Dark Sparkle</a>
      </li>
      <li>
        <a href="/win&FX=22" target="controlframe">Dark Sparkle+</a>
      </li>
      <li>
        <a href="/win&FX=23" target="controlframe">Strobe</a>
      </li>
      <li>
        <a href="/win&FX=24" target="controlframe">Strobe Rainbow</a>
      </li>
      <li>
        <a href="/win&FX=25" target="controlframe">Double Strobe</a>
      </li>
      <li>
        <a href="/win&FX=26" target="controlframe">Blink Rainbow</a>
      </li>
      <li>
        <a href="/win&FX=27" target="controlframe">Chase+</a>
      </li>
      <li>
        <a href="/win&FX=28" target="controlframe">Dark Chase</a>
      </li>
      <li>
        <a href="/win&FX=29" target="controlframe">Dark Chase Random</a>
      </li>
      <li>
        <a href="/win&FX=30" target="controlframe">Dark Chase Rainbow</a>
      </li>
      <li>
        <a href="/win&FX=31" target="controlframe">Chase Flash</a>
      </li>
      <li>
        <a href="/win&FX=32" target="controlframe">Dark Chase Random</a>
      </li>
      <li>
        <a href="/win&FX=33" target="controlframe">Rainbow Runner</a>
      </li>
      <li>
        <a href="/win&FX=34" target="controlframe">Colourful</a>
      </li>
      <li>
        <a href="/win&FX=35" target="controlframe">Traffic Light</a>
      </li>
      <li>
        <a href="/win&FX=36" target="controlframe">Sweep Random</a>
      </li>
      <li>
        <a href="/win&FX=37" target="controlframe">Running 2</a>
      </li>
      <li>
        <a href="/win&FX=38" target="controlframe">Red & Blue</a>
      </li>
      <li>
        <a href="/win&FX=39" target="controlframe">Running 2 Random</a>
      </li>
      <li>
        <a href="/win&FX=40" target="controlframe">Scanner</a>
      </li>
      <li>
        <a href="/win&FX=41" target="controlframe">Lighthouse</a>
      </li>
      <li>
        <a href="/win&FX=42" target="controlframe">Fireworks</a>
      </li>
      <li>
        <a href="/win&FX=43" target="controlframe">Fireworks Random</a>
      </li>
      <li>
        <a href="/win&FX=44" target="controlframe">Merry Christmas</a>
      </li>
      <li>
        <a href="/win&FX=45" target="controlframe">Fire Flicker</a>
      </li>
      <li>
        <a href="/win&FX=46" target="controlframe">Gradient</a>
      </li>
      <li>
        <a href="/win&FX=47" target="controlframe">Gradient Loading</a>
      </li>
      <li>
        <a href="/win&FX=48" target="controlframe">In Out</a>
      </li>
      <li>
        <a href="/win&FX=49" target="controlframe">In In</a>
      </li>
      <li>
        <a href="/win&FX=50" target="controlframe">Out Out</a>
      </li>
      <li>
        <a href="/win&FX=51" target="controlframe">Out In</a>
      </li>
      <li>
        <a href="/win&FX=52" target="controlframe">Circus</a>
      </li>
      <li>
        <a href="/win&FX=53" target="controlframe">New Chase</a>
      </li>
      <li>
        <a href="/win&FX=54" target="controlframe">New Chase Rainbow Loop</a>
      </li>
      <li>
        <a href="/win&FX=55" target="controlframe">New Chase Rainbow</a>
      </li>
      <li>
        <a href="/win&FX=56" target="controlframe">New Chase Blink</a>
      </li>
      <li>
        <a href="/win&FX=57" target="controlframe">Random Chaos</a>
      </li>
    </ul><iframe id="stf" name="controlframe" onload="feedback();" style="display:none;"></iframe>
  </main>
  <script>
         /*!
          * iro.js v3.3.0
          * 2016-2018 James Daniel
          * Released under the MIT license
          * github.com/jaames/iro.js
          */
  !function(t,e){"object"==typeof exports&&"object"==typeof module?module.exports=e():"function"==typeof define&&define.amd?define([],e):"object"==typeof exports?exports.iro=e():t.iro=e()}(this,function(){return function(t){function e(s){if(r[s])return r[s].exports;var n=r[s]={i:s,l:!1,exports:{}};return t[s].call(n.exports,n,n.exports,e),n.l=!0,n.exports}var r={};return e.m=t,e.c=r,e.i=function(t){return t},e.d=function(t,r,s){e.o(t,r)||Object.defineProperty(t,r,{configurable:!1,enumerable:!0,get:s})},e.n=function(t){var r=t&&t.t?function(){return t.default}:function(){return t};return e.d(r,"a",r),r},e.o=function(t,e){return Object.prototype.hasOwnProperty.call(t,e)},e.p="",e(e.s=4)}([function(t,e,r){"use strict";function s(t){var e,r,s,n,i,o,a,h,c=t.h/360,u=t.s/100,l=t.v/100;switch(n=k(6*c),i=6*c-n,o=l*(1-u),a=l*(1-i*u),h=l*(1-(1-i)*u),n%6){case 0:e=l,r=h,s=o;break;case 1:e=a,r=l,s=o;break;case 2:e=o,r=l,s=h;break;case 3:e=o,r=a,s=l;break;case 4:e=h,r=o,s=l;break;case 5:e=l,r=o,s=a}return{r:w(255*e),g:w(255*r),b:w(255*s)}}function n(t){var e,r=t.r/255,s=t.g/255,n=t.b/255,i=Math.max(r,s,n),o=Math.min(r,s,n),a=i-o;switch(i){case o:e=0;break;case r:e=(s-n)/a+(s<n?6:0);break;case s:e=(n-r)/a+2;break;case n:e=(r-s)/a+4}return e/=6,{h:w(360*e),s:w(0==i?0:a/i*100),v:w(100*i)}}function i(t){var e=t.s/100,r=t.v/100,s=.5*r*(2-e);return e=r*e/(1-Math.abs(2*s-1)),{h:t.h,s:w(100*e)||0,l:w(100*s)}}function o(t){var e=t.s/100,r=t.l/100;return r*=2,e*=r<=1?r:2-r,{h:t.h,s:w(2*e/(r+e)*100),v:w((r+e)/2*100)}}function a(t){return"rgb"+(t.a?"a":"")+"("+t.r+", "+t.g+", "+t.b+(t.a?", "+t.a:"")+")"}function h(t){return"hsl"+(t.a?"a":"")+"("+t.h+", "+t.s+"%, "+t.l+"%"+(t.a?", "+t.a:"")+")"}function c(t){var e=t.r,r=t.g,s=t.b,n=e%17==0&&r%17==0&&s%17==0,i=n?17:1,o=n?4:8,a=n?4:7,h=e/i<<2*o|r/i<<o|s/i,c=h.toString(16);return"#"+new Array(a-c.length).join("0")+c}function u(t,e){var r=t.match(/(\S+)\((\d+)(%?)(?:\D+?)(\d+)(%?)(?:\D+?)(\d+)(%?)(?:\D+?)?([0-9\.]+?)?\)/i),s=parseInt(r[2]),n=parseInt(r[4]),i=parseInt(r[6]);return[r[1],"%"==r[3]?s/100*e[0]:s,"%"==r[5]?n/100*e[1]:n,"%"==r[7]?i/100*e[2]:i,parseFloat(r[8])||void 0]}function l(t){var e=u(t,[255,255,255]);return{r:e[1],g:e[2],b:e[3]}}function f(t){var e=u(t,[360,100,100]);return{h:e[2],s:e[3],l:e[4]}}function v(t){t=t.replace("#","");var e=parseInt("0x"+t),r=3==t.length,s=r?15:255,n=r?4:8,i=r?17:1;return{r:(e>>2*n&s)*i,g:(e>>n&s)*i,b:(e&s)*i}}function p(t){return t instanceof x?t:new x(t)}function d(t,e,r){return t<=e?e:t>=r?r:t}function g(t,e){var r={};for(var s in t)r[s]=e[s]!=t[s];return r}function _(t,e,r){var s=p(t).rgb,n=p(e).rgb;return r=d(r/100||.5,0,1),new x({r:k(s.r+(n.r-s.r)*r),g:k(s.g+(n.g-s.g)*r),b:k(s.b+(n.b-s.b)*r)})}function m(t,e){var r=p(t),s=r.hsv;return s.v=d(s.v+e,0,100),r.hsv=s,r}function y(t,e){var r=p(t),s=r.hsv;return s.v=d(s.v-e,0,100),r.hsv=s,r}var b="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function(t){return typeof t}:function(t){return t&&"function"==typeof Symbol&&t.constructor===Symbol&&t!==Symbol.prototype?"symbol":typeof t},w=Math.round,k=Math.floor,x=function(t){this.e=!1,this.u={h:void 0,s:void 0,v:void 0},t&&this.set(t)};x.mix=_,x.lighten=m,x.darken=y,x.hsv2Rgb=s,x.rgb2Hsv=n,x.hsv2Hsl=i,x.hsl2Hsv=o,x.hsl2Str=h,x.rgb2Str=a,x.rgb2Hex=c,x.parseHexStr=v,x.parseHslStr=f,x.parseRgbStr=l,x.prototype={constructor:x,set:function(t){"object"==(void 0===t?"undefined":b(t))?t instanceof x?this.hsv=x.hsv:"r"in t?this.rgb=t:"v"in t?this.hsv=t:"l"in t&&(this.hsl=t):"string"==typeof t&&(/^rgb/.test(t)?this.rgbString=t:/^hsl/.test(t)?this.hslString=t:/^#[0-9A-Fa-f]/.test(t)&&(this.hexString=t))},setChannel:function(t,e,r){var s=this[t];s[e]=r,this[t]=s},clone:function(){return new x(this)},compare:function(t,e){return e=e||"hsv",g(this[e],p(t)[e])},mix:function(t,e){this.hsv=_(this,t,e).hsv},lighten:function(t){m(this,t)},darken:function(t){y(this,t)}},Object.defineProperties(x.prototype,{hsv:{get:function(){var t=this.u;return{h:t.h,s:t.s,v:t.v}},set:function(t){if(this.e){var e=this.u;for(var r in e)t.hasOwnProperty(r)||(t[r]=e[r]);var s=g(e,t);this.u=t,(s.h||s.s||s.v)&&this.e(this,s)}else this.u=t}},rgb:{get:function(){return s(this.u)},set:function(t){this.hsv=n(t)}},hsl:{get:function(){return i(this.u)},set:function(t){this.hsv=o(t)}},rgbString:{get:function(){return a(this.rgb)},set:function(t){this.rgb=l(t)}},hexString:{get:function(){return c(this.rgb)},set:function(t){this.rgb=v(t)}},hslString:{get:function(){return h(this.hsl)},set:function(t){this.hsl=x.parseHslStr(t)}}}),t.exports=x},function(t,e,r){"use strict";var s=function(){var t=document.createElement("style");document.head.appendChild(t),t.appendChild(document.createTextNode("")),this.style=t;var e=t.sheet;this.sheet=e,this.rules=e.rules||e.cssRules,this.map={}};s.prototype={constructor:s,setRule:function(t,e,r){var s=this.sheet,n=s.rules||s.cssRules,i=this.map;if(e=e.replace(/([A-Z])/g,function(t){return"-"+t.toLowerCase()}),i.hasOwnProperty(t))i[t].setProperty(e,r);else{var o=n.length,a=e+": "+r;try{s.insertRule(t+" {"+a+";}",o)}catch(e){s.addRule(t,a,o)}finally{n=s.rules||s.cssRules,i[t]=n[o].style}}}},Object.defineProperties(s.prototype,{enabled:{get:function(){return!this.sheet.disabled},set:function(t){this.sheet.disabled=!t}},cssText:{get:function(){var t=this.map,e=[];for(var r in t)e.push(r.replace(/,\W/g,",\n")+" {\n\t"+t[r].cssText.replace(/;\W/g,";\n\t")+"\n}");return e.join("\n")}},css:{get:function(){var t=this.map,e={};for(var r in t){var s=t[r];e[r]={};for(var n=0;n<s.length;n++){var i=s[n];e[r][i]=s.getPropertyValue(i)}}return e}}}),t.exports=s},function(t,e,r){"use strict";var s=function(t,e){var r=t.g({f:"iro__marker"});r._(0,0,e.r,{f:"iro__marker__outer",fill:"none",k:5,M:"#000"}),r._(0,0,e.r,{f:"iro__marker__inner",fill:"none",k:2,M:"#fff"}),this.g=r};s.prototype={constructor:s,move:function(t,e){this.g.S("translate",[t,e])}},t.exports=s},function(t,e,r){"use strict";function s(t){return t&&t.t?t:{default:t}}function n(t,e,r){for(var s=0;s<e.length;s++)t.addEventListener(e[s],r)}function i(t,e,r){for(var s=0;s<e.length;s++)t.removeEventListener(e[s],r)}function o(t){document.readyState==m?t():n(document,[_],function e(r){document.readyState==m&&(t(),i(document,[_],e))})}var a=r(7),h=s(a),c=r(5),u=s(c),l=r(6),f=s(l),v=r(0),p=s(v),d=r(1),g=s(d),_="readystatechange",m="complete",y=function(t,e){var r=this;e=e||{},this.C={},this.T=!1,this.A=!1,this.css=e.css||e.styles||void 0,o(function(){r.j(t,e)})};y.prototype={constructor:y,j:function(t,e){var r=this;t="string"==typeof t?document.querySelector(t):t;var s=e.width||parseInt(t.width)||320,i=e.height||parseInt(t.height)||320,o=e.padding+2||6,a=e.borderWidth||0,c=e.markerRadius||8,l=e.sliderMargin||24,v=e.sliderHeight||2*c+2*o+2*a,d=Math.min(i-v-l,s),_=d/2-a,m=(s-d)/2,y={r:c},b={w:a,color:e.borderColor||"#fff"};this.el=t,this.svg=new f.default(t,s,i,e.display),this.ui=[new h.default(this.svg,{cX:m+d/2,cY:d/2,r:_,rMax:_-(c+o),marker:y,border:b,lightness:void 0==e.wheelLightness||e.wheelLightness,anticlockwise:e.anticlockwise}),new u.default(this.svg,{sliderType:"v",x:m+a,y:d+l,w:d-2*a,h:v-2*a,r:v/2-a,marker:y,border:b})],this.stylesheet=new g.default,this.color=new p.default,this.color.e=this.H.bind(this),this.color.set(e.color||e.defaultValue||"#fff"),this.on("history:stateChange",function(t){r.svg.updateUrls(t)}),n(this.svg.el,["mousedown","touchstart"],this)},H:function(t,e){for(var r=t.rgbString,s=this.css,n=0;n<this.ui.length;n++)this.ui[n].update(t,e);for(var i in s){var o=s[i];for(var a in o)this.stylesheet.setRule(i,a,r)}this.A||(this.A=!0,this.emit("color:change",t,e),this.A=!1)},on:function(t,e){var r=this.C;(r[t]||(r[t]=[])).push(e)},off:function(t,e){var r=this.C[t];r&&r.splice(r.indexOf(e),1)},emit:function(t){for(var e=this.C,r=(e[t]||[]).concat(e["*"]||[]),s=arguments.length,n=Array(s>1?s-1:0),i=1;i<s;i++)n[i-1]=arguments[i];for(var o=0;o<r.length;o++)r[o].apply(null,n)},handleEvent:function(t){var e=t.touches?t.changedTouches[0]:t,r=this.svg.el.getBoundingClientRect(),s=e.clientX-r.left,o=e.clientY-r.top;switch(t.type){case"mousedown":case"touchstart":for(var a=0;a<this.ui.length;a++){var h=this.ui[a];h.checkHit(s,o)&&(this.T=h,n(document,["mousemove","touchmove","mouseup","touchend"],this),this.emit("input:start"),this.color.hsv=this.T.input(s,o))}break;case"mousemove":case"touchmove":this.color.hsv=this.T.input(s,o);break;case"mouseup":case"touchend":this.T=!1,this.emit("input:end"),i(document,["mousemove","touchmove","mouseup","touchend"],this)}this.T&&t.preventDefault()}},t.exports=y},function(t,e,r){"use strict";function s(t){return t&&t.t?t:{default:t}}var n=r(3),i=s(n),o=r(0),a=s(o),h=r(1),c=s(h);t.exports={Color:a.default,ColorPicker:i.default,Stylesheet:c.default,version:"3.3.0"}},function(t,e,r){"use strict";function s(t){return t&&t.t?t:{default:t}}var n=r(2),i=s(n),o=r(0),a=s(o),h=function(t,e){var r=e.r,s=e.w,n=e.h,o=e.x,a=e.y,h=e.border.w;e.range={min:o+r,max:o+s-r,w:s-2*r},e.sliderType=e.sliderType||"v",this.type="slider",this.P=e;var c=r+h/2,u=t.g({f:"iro__slider"}),l=u.R("rect",{f:"iro__slider__value",rx:c,ry:c,x:o-h/2,y:a-h/2,width:s+h,height:n+h,k:h,M:e.border.color});l.setGradient("fill",t.O("linear",{0:{color:"#000"},100:{color:"#fff"}})),this.I=l.O,this.marker=new i.default(u,e.marker)};h.prototype={constructor:h,update:function(t,e){var r=this.P,s=r.range,n=t.hsv,i=a.default.hsv2Hsl({h:n.h,s:n.s,v:100});if("v"==r.sliderType&&((e.h||e.s)&&this.I.stops[1].W({L:"hsl("+i.h+","+i.s+"%,"+i.l+"%)"}),e.v)){var o=n.v/100;this.marker.move(s.min+o*s.w,r.y+r.h/2)}},input:function(t,e){var r=this.P,s=r.range,n=Math.max(Math.min(t,s.max),s.min)-s.min;return{v:Math.round(100/s.w*n)}},checkHit:function(t,e){var r=this.P;return t>r.x&&t<r.x+r.w&&e>r.y&&e<r.y+r.h}},t.exports=h},function(t,e,r){"use strict";var s=0,n={f:"class",M:"stroke",k:"stroke-width",fill:"fill",G:"opacity",X:"offset",L:"stop-color",Y:"stop-opacity"},i={translate:"setTranslate",scale:"setScale",rotate:"setRotate"},o=window.navigator.userAgent.toLowerCase(),a=/msie|trident|edge/.test(o),h=/^((?!chrome|android).)*safari/i.test(o),c=function(t,e,r,s){var n=document.createElementNS("http://www.w3.org/2000/svg",r);this.el=n,this.W(s),(e.el||e).appendChild(n),this.U=t,this.D={},this.V=!!n.transform&&n.transform.baseVal};c.prototype={constructor:c,R:function(t,e){return new c(this.U,this,t,e)},g:function(t){return this.R("g",t)},F:function(t,e,r,s,n,i){var o=n-s<=180?0:1;s*=Math.PI/180,n*=Math.PI/180;var a=t+r*Math.cos(n),h=e+r*Math.sin(n),c=t+r*Math.cos(s),u=e+r*Math.sin(s);return i=i||{},i.d=["M",a,h,"A",r,r,0,o,0,c,u].join(" "),this.R("path",i)},_:function(t,e,r,s){return s=s||{},s.cx=t,s.cy=e,s.r=r,this.R("circle",s)},S:function(t,e){if(a)this.W({transform:t+"("+e.join(", ")+")"});else{var r,s,n=this.D;n[t]?r=n[t]:(r=this.U.el.createSVGTransform(),n[t]=r,this.V.appendItem(r)),s=t in i?i[t]:t,r[s].apply(r,e)}},W:function(t){for(var e in t){var r=e in n?n[e]:e;this.el.setAttribute(r,t[e])}},setGradient:function(t,e){var r={};r[t]=e.getUrl(),e.q[t]=this,this.O=e,this.W(r)}};var u=function(t,e,r){var n=[],i=t.N.R(e+"Gradient",{id:"iroGradient"+s++});for(var o in r){var a=r[o];n.push(i.R("stop",{X:o+"%",L:a.color,Y:void 0===a.G?1:a.G}))}this.el=i.el,this.stops=n,this.q={}};u.prototype.getUrl=function(t){return"url("+(h?t||window.location.href:"")+"#"+this.el.id+")"};var l=function(t,e,r,s){c.call(this,this,t,"svg",{width:e,height:r,style:"display:"+(s||"block")}),this.N=this.R("defs"),this.B=[]};l.prototype=Object.create(c.prototype),l.prototype.constructor=l,l.prototype.O=function(t,e){var r=new u(this,t,e);return this.B.push(r),r},l.prototype.updateUrls=function(t){if(h)for(var e=this.B,r=0;r<e.length;r++)for(var s in e[r].q){var n={};n[s]=e[r].getUrl(t),e[r].q[s].W(n)}},t.exports=l},function(t,e,r){"use strict";var s=r(2),n=function(t){return t&&t.t?t:{default:t}}(s),i=Math.PI,o=Math.sqrt,a=Math.abs,h=Math.round,c=function(t,e){this.P=e,this.type="wheel";var r=e.cY,s=e.cX,i=e.r,o=e.border,a=t.g({f:"iro__wheel"});a._(s,r,i+o.w/2,{f:"iro__wheel__border",fill:"#fff",M:o.color,k:o.w});for(var h=a.g({f:"iro__wheel__hue",k:i,fill:"none"}),c=0;c<360;c++)h.F(s,r,i/2,c,c+1.5,{M:"hsl("+(e.anticlockwise?360-c:c)+",100%,50%)"});a._(s,r,i,{f:"iro__wheel__saturation"}).setGradient("fill",t.O("radial",{0:{color:"#fff"},100:{color:"#fff",G:0}})),this.Z=a._(s,r,i,{f:"iro__wheel__lightness",G:0}),this.marker=new n.default(a,e.marker)};c.prototype={constructor:c,update:function(t,e){var r=this.P,s=t.hsv;if(e.v&&r.lightness&&this.Z.W({G:(1-s.v/100).toFixed(2)}),e.h||e.s){var n=(r.anticlockwise?360-s.h:s.h)*(i/180),o=s.s/100*r.rMax;this.marker.move(r.cX+o*Math.cos(n),r.cY+o*Math.sin(n))}},input:function(t,e){var r=this.P,s=r.rMax,n=r.cX-t,a=r.cY-e,c=Math.atan2(a,n),u=h(c*(180/i))+180,l=Math.min(o(n*n+a*a),s);return u=r.anticlockwise?360-u:u,{h:u,s:h(100/s*l)}},checkHit:function(t,e){var r=this.P,s=a(t-r.cX),n=a(e-r.cY);return o(s*s+n*n)<r.r}},t.exports=c}])});
  </script> 
  <script>
     //Feedback
       function feedback(){
       var e = document.body;
         
       e.classList.remove("feedbackanim");
       void e.offsetWidth;
       e.classList.add("feedbackanim");
     }
      
   
     
     //Speed slider
     document.getElementById('speedslider').onchange = function() {
         var speedurl = '/win&SX=' + this.value;
         LEDCommand(speedurl);
     }
         //Brightness slider
     document.getElementById('brightslider').onchange = function() {
         var brighturl = '/win&A=' + this.value;
         LEDCommand(brighturl);
     }
         //Preset speed slider
     document.getElementById('cyclespeedslider').onchange = function() {
         var val = this.value;
         var url = '/win&PT=' + val;
         var seconds =  millisToMinutesAndSeconds(val);
         LEDCommand(url);
         document.getElementById('PSDurationLabel').innerHTML = 'Preset Duration (' + seconds + ')';
     }
         //Cycle Transition slider
     document.getElementById('cycletransitionslider').onchange = function() {
         var val = this.value;
         var seconds = millisToMinutesAndSeconds(val);
         var url = '/win&TT=' + this.value;
         document.getElementById('CDurationLabel').innerHTML = 'Color Transition (' + seconds + ')';
         LEDCommand(url);
     }
  var demoColorPicker = new iro.ColorPicker("#color-picker-container", {
     // Set the size of the color picker UI
     width: 280,
     height: 280,
     // Set the initial color to red
     color: "#FFF"
  });
  function toggleCycle(){
   if (document.getElementById('myonoffswitch-cy').checked) 
   {
       LEDCommand('win&CY=1');
   } else {
       LEDCommand('win&CY=0');
   }
  }
  function millisToMinutesAndSeconds(millis) {
  var minutes = Math.floor(millis / 60000);
  var seconds = ((millis % 60000) / 1000).toFixed(0);
  return minutes + ":" + (seconds < 10 ? '0' : '') + seconds;
}
  window.onload = function () { 
  demoColorPicker.on("color:change", function(color, changes) {
     // Log the color's hex RGB value to the dev console
     console.log(color.rgb);
     //console.log("r = " + color.rgb.r);
     var cr = color.rgb.r;
     var cg = color.rgb.g;
     var cb = color.rgb.b;
     //Secondary color
     if (document.querySelector('.onoffswitch-checkbox').checked == true) {
         var rgburl = "/win&R2=" + cr + "&G2=" + cg + "&B2=" + cb;
         LEDCommand(rgburl);
     }
     //Primary color
     if (document.querySelector('.onoffswitch-checkbox').checked == false) {
         var rgburl = "/win&R=" + cr + "&G=" + cg + "&B=" + cb;
         LEDCommand(rgburl);
     }
  });
  // Initialize Sliders
   var sliderSections = document.getElementsByClassName("range-slider");
       for( var x = 0; x < sliderSections.length; x++ ){
         var sliders = sliderSections[x].getElementsByTagName("input");
         for( var y = 0; y < sliders.length; y++ ){
           if( sliders[y].type ==="range" ){
             sliders[y].oninput = getVals;
             // Manually trigger event first time to display values
             sliders[y].oninput();
           }
         }
       }
  }
  function LEDCommand(rgburl){
   document.getElementById('stf').src = rgburl;
   console.log(rgburl);
  }
  function presetColour(colour) {
     var pc = iro.Color.parseHexStr(colour);
     console.log(pc);
     var cr = pc.r;
     var cg = pc.g;
     var cb = pc.b;
     if (document.querySelector('.onoffswitch-checkbox').checked == true) {
         var rgburl = "/win&R2=" + cr + "&G2=" + cg + "&B2=" + cb;
     }
     if (document.querySelector('.onoffswitch-checkbox').checked == false) {
         var rgburl = "/win&R=" + cr + "&G=" + cg + "&B=" + cb;
     }
     LEDCommand(rgburl);
  }
  function togglePower(){
   var s = document.getElementById("brightslider");
   if (s.value > 20){
     s.value = 0;
     LEDCommand('/win&A=' + s.value);
   }
   else
   {
     s.value = 250;
     LEDCommand('/win&A=' + s.value);
   }
  }
  function toggleNight(){
   LEDCommand('/win&ND');
   alert("Night Light Activated. Check settings for details.");
  }
  function preset(presetNum){
   if (document.querySelector('.onoffswitch-checkbox-ps').checked == false) {
         var url = "/win&PL=" + presetNum;
     }
   if (document.querySelector('.onoffswitch-checkbox-ps').checked == true) {
         var url = "/win&PS=" + presetNum;
     }
   LEDCommand(url);
  }
  function getVals(){
   // Get slider values
   var parent = this.parentNode;
   var slides = parent.getElementsByTagName("input");
     var slide1 = parseFloat( slides[0].value );
     var slide2 = parseFloat( slides[1].value );
   // Neither slider will clip the other, so make sure we determine which is larger
   if( slide1 > slide2 ){ var tmp = slide2; slide2 = slide1; slide1 = tmp; }
   
   var displayElement = parent.getElementsByClassName("rangeValues")[0];
       displayElement.innerHTML = slide1 + " - " + slide2;
   LEDCommand('/win&P1=' + slide1 + '&P2=' + slide2);    
  }
  //Preset Gradients
  function presetGradient(color1, color2){
   
  }
  //Hopefully Gradient Colors soon...
  function gradientBtns(color1, color2, color3, color4, color5) {
     document.getElementById("gradients").innerHTML = document.getElementById("gradients").innerHTML +
         '<div onclick="presetGradient("' + color1 + ', ' + color2 + '");" class="colourCol" style="background: -moz-linear-gradient(left, ' + color1 + ' 0%, ' + color2 + ' 100%); background: -webkit-linear-gradient(left, ' + color1 + ' 0%,' + color2 + ' 100%); background: linear-gradient(to right, ' + color1 + ' 0%,' + color2 + '100%); filter: progid:DXImageTransform.Microsoft.gradient( startColorstr="' + color1 + '", endColorstr="' + color2 + '",GradientType=1 );>&nbsp;<\/div>';
  }
  </script>
</body>
</html>
)=====";
/*
 * Classic UI Index html
 */
//head0 (js)
const char PAGE_index0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><meta charset=utf-8>
<link rel='shortcut icon' type=image/x-icon href='/favicon.ico'/>
<title>WLED 0.7.0</title>
<script>var d=document;var w=window.getComputedStyle(d.querySelector("html"));resp="";var nla=false;var nra=false;var nsa=false;var sto=false;var uwv=false;var sbf=true;var firstload=true;var lastsx=0;var nState=0;var cv=0;var lm=0;aC="";bC="";dC="";function gId(s){return d.getElementById(s)}function Startup(){var w=window.getComputedStyle(d.querySelector("html"));aC=w.getPropertyValue("--aCol");bC=w.getPropertyValue("--bCol");dC=w.getPropertyValue("--dCol");CV(0);setInterval("GIO()",5E3);GIO()}function GIO(){nocache="&nocache="+Math.random()*1E6;var request=new XMLHttpRequest;request.onreadystatechange=function(){if(this.readyState==4)if(this.status==200)if(this.responseXML!=null){d.Cf.SA.value=this.responseXML.getElementsByTagName("ac")[0].childNodes[0].nodeValue;d.Cf.SR.value=this.responseXML.getElementsByTagName("cl")[0].childNodes[0].nodeValue;d.Cf.SG.value=this.responseXML.getElementsByTagName("cl")[1].childNodes[0].nodeValue;d.Cf.SB.value=this.responseXML.getElementsByTagName("cl")[2].childNodes[0].nodeValue;if(this.responseXML.getElementsByTagName("wv")[0].childNodes[0].nodeValue>=0){d.Cf.SW.value=this.responseXML.getElementsByTagName("wv")[0].childNodes[0].nodeValue;uwv=true}else uwv=false;Cf.TX.selectedIndex=this.responseXML.getElementsByTagName("fx")[0].childNodes[0].nodeValue;d.Cf.SX.value=this.responseXML.getElementsByTagName("sx")[0].childNodes[0].nodeValue;d.Cf.IX.value=this.responseXML.getElementsByTagName("ix")[0].childNodes[0].nodeValue;nla=this.responseXML.getElementsByTagName("nl")[0].innerHTML!=0?true:false;if(firstload){d.Cf.NC.checked=this.responseXML.getElementsByTagName("nf")[0].innerHTML!=0?true:false;d.Cf.SN.value=this.responseXML.getElementsByTagName("nd")[0].childNodes[0].nodeValue;d.Cf.ST.value=this.responseXML.getElementsByTagName("nt")[0].childNodes[0].nodeValue;CV(parseInt(this.responseXML.getElementsByTagName("md")[0].childNodes[0].nodeValue))}firstload=false;nState=0;nState=this.responseXML.getElementsByTagName("nr")[0].innerHTML!=0?1:0;nState+=this.responseXML.getElementsByTagName("ns")[0].innerHTML!=0?2:0;d.getElementsByClassName("desc")[0].innerHTML=this.responseXML.getElementsByTagName("ds")[0].innerHTML;UV()}};request.open("GET","win"+resp+nocache,true);request.send(null);resp=""}function GC(){resp+="&A="+Cf.SA.value;resp+="&R="+Cf.SR.value;resp+="&G="+Cf.SG.value;resp+="&B="+Cf.SB.value;if(uwv)resp+="&W="+Cf.SW.value;UV();GIO()}function GX(){resp+="&FX="+Cf.TX.selectedIndex;resp+="&SX="+Cf.SX.value;resp+="&IX="+Cf.IX.value;UV();GIO()}function GetRGB(){var r,g,b,i,f,p,q,t;var h=d.Cf.SH.value,s=d.Cf.SS.value,v=255;i=Math.floor(h*6);f=h*6-i;p=v*(1-s);q=v*(1-f*s);t=v*(1-(1-f)*s);switch(i%6){case 0:r=v,g=t,b=p;break;case 1:r=q,g=v,b=p;break;case 2:r=p,g=v,b=t;break;case 3:r=p,g=q,b=v;break;case 4:r=t,g=p,b=v;break;case 5:r=v,g=p,b=q}d.Cf.SR.value=r;d.Cf.SG.value=g;d.Cf.SB.value=b;GC()}function GetCC(){resp+="&CP=";resp+=d.Cf.PF.value;resp+="&CS=";resp+=d.Cf.SF.value;resp+="&CM=";resp+=d.Cf.HF.value;resp+=d.Cf.SC.checked?"&CF=1":"&CF=0";resp+=d.Cf.EC.checked?"&CE=1":"&CE=0";GIO()}function CV(v){if(sto)CloseSettings();gId("slA").style.display="none";gId("srgb").style.display="none";gId("shs").style.display="none";gId("slW").style.display="none";gId("tlX").style.display="none";gId("tlP").style.display="none";gId("tlN").style.display="none";if(v<2){if(uwv)gId("slW").style.display="block";gId("slA").style.display="block"}switch(v){case 0:gId("srgb").style.display="block";lm=0;break;case 1:gId("shs").style.display="block";lm=1;break;case 2:gId("tlP").style.display="block";break;case 3:gId("tlX").style.display="block";break;case 4:gId("tlN").style.display="block"}cv=v;mdb.style.fill=lm>0?aC:dC}function rgb2hex(red,green,blue){var rgb=blue|green<<8|red<<16;return"#"+(16777216+rgb).toString(16).slice(1)}function lingrad(r,g,b){return"linear-gradient("+bC+","+rgb2hex(r,g,b)+")"}function UV(){d.body.style.background=lingrad(Cf.SR.value,Cf.SG.value,Cf.SB.value);setHS(Cf.SR.value,Cf.SG.value,Cf.SB.value);fxb.style.fill=d.Cf.TX.selectedIndex>0?aC:dC;nlb.style.fill=nla?aC:dC;ntb.style.fill=nla?aC:dC;switch(nState){case 0:gId("path1").style.fill=dC;gId("path2").style.fill=dC;break;case 1:gId("path1").style.fill=aC;gId("path2").style.fill=dC;break;case 2:gId("path1").style.fill=dC;gId("path2").style.fill=aC;break;case 3:gId("path1").style.fill=aC;gId("path2").style.fill=aC}tgb.style.fill=Cf.SA.value>0?aC:dC;ccX.style.display=Cf.TX.selectedIndex>52?"block":"none";fof.style.fill=Cf.TX.selectedIndex>52?aC:dC;fmr.style.fill=Cf.TX.selectedIndex<1?aC:dC}function TgT(){if(Cf.SA.value>0){resp+="&T=0";Cf.SA.value=0}else resp+="&T=2";UV();GIO()}function SwFX(s){var n=Cf.TX.selectedIndex+s;Cf.TX.selectedIndex=n;if(n<0)Cf.TX.selectedIndex=0;if(n>57)Cf.TX.selectedIndex=53;GX()}function TgHSB(){if(cv<2)cv?CV(0):CV(1);else CV(lm)}function SwitchPS(x){d.Cf.FF.value=parseInt(d.Cf.FF.value)+x;if(d.Cf.FF.value<1)d.Cf.FF.value=1;if(d.Cf.FF.value>25)d.Cf.FF.value=25}function PAt(){resp+=d.Cf.BC.checked?"&PA=1":"&PA=0";resp+=d.Cf.CC.checked?"&PC=1":"&PC=0";resp+=d.Cf.FC.checked?"&PX=1":"&PX=0"}function PSIO(sv){PAt();if(sv){resp+="&PS=";resp+=d.Cf.FF.value}else{resp+="&PL=";resp+=d.Cf.FF.value}GIO()}function OpenSettings(){sto=true;stb.style.fill=aC;cdB.style.display="none";stf.style.display="inline";if(sbf)stf.src="/settings";sbf=false}function CloseSettings(){sto=false;stb.style.fill=dC;cdB.style.display="inline";stf.style.display="none"}function TgS(){if(sto)CloseSettings();else OpenSettings()}function TgNl(){nla=!nla;if(nla){resp+="&NL="+d.Cf.SN.value;resp+="&NT="+d.Cf.ST.value;resp+=d.Cf.NC.checked?"&NF=1":"&NF=0"}else resp+="&NL=0";UV();GIO()}function TgN(){nState++;if(nState>3)nState=0;switch(nState){case 0:resp+="&SN=0&RN=0";break;case 1:resp+="&SN=0&RN=1";break;case 2:resp+="&SN=1&RN=0";break;case 3:resp+="&SN=1&RN=1"}UV();GIO()}function setHS(){var rr,gg,bb,r=arguments[0]/255,g=arguments[1]/255,b=arguments[2]/255,h,s,v=Math.max(r,g,b),diff=v-Math.min(r,g,b),diffc=function(c){return(v-c)/6/diff+1/2};if(diff==0)h=s=0;else{s=diff/v;rr=diffc(r);gg=diffc(g);bb=diffc(b);if(r===v)h=bb-gg;else if(g===v)h=1/3+rr-bb;else if(b===v)h=2/3+gg-rr;if(h<0)h+=1;else if(h>1)h-=1}if(s>0)d.Cf.SH.value=h;d.Cf.SS.value=s}function CS(i){switch(i){case 0:resp+="&SW";break;case 1:resp+="&SB";break;case 2:resp+="&SR=1";break;case 3:resp+="&SP";break;case 4:resp+="&SC";break;case 5:resp+="&SR=0"}GIO()}function uCY(){PAt();resp+=d.Cf.CY.checked?"&CY=1":"&CY=0";resp+="&P1="+Cf.P1.value;resp+="&P2="+Cf.P2.value;resp+="&PT="+Cf.PT.value;GIO()}function R(){resp+="&PL=0";GIO()};</script>
)=====";

//head1 (css)
const char PAGE_index1[] PROGMEM = R"=====(
.ctrl_box{margin:auto;width:80vw;background-color:var(--cCol);position:absolute;top:55%;left:50%;transform:translate(-50%,-50%);filter:drop-shadow(-5px -5px 5px var(--sCol))}.sds{width:100%;height:12vh;margin-top:2vh}.sl{margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw}#slA{margin-top:0vh;background:linear-gradient(to right,black,yellow)}#slR{background:linear-gradient(to right,black,red)}#slG{background:linear-gradient(to right,black,green)}#slB{background:linear-gradient(to right,black,blue)}#slW{background:linear-gradient(to right,black,white)}#slH{background:linear-gradient(to right,red,orange,yellow,green,cyan,blue,violet,red)}#slS{background:linear-gradient(to right,grey,green)}#slN{background:linear-gradient(to right,black,turquoise)}#slT{background:linear-gradient(to right,black,yellow)}.tools{margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw}#slX{background:linear-gradient(to right,black,white)}#slI{background:linear-gradient(to right,black,red)}body{font-family:var(--cFn),sans-serif;text-align:center;background:linear-gradient(var(--bCol),black);height:100%;margin:0;background-repeat:no-repeat;background-attachment:fixed;color:var(--tCol)}html{height:100%}iframe{display:none;border:0;filter:drop-shadow(-5px -5px 5px var(--sCol));margin:auto;width:80vw;height:60vh;position:absolute;top:55%;left:50%;transform:translate(-50%,-50%)}svg{fill:var(--dCol);width:12vw;height:10vmin;filter:drop-shadow(-5px -5px 5px var(--sCol))}input{filter:drop-shadow(-5px -5px 5px var(--sCol));}button{background:var(--bCol);color:var(--tCol);border:.5ch solid var(--bCol);margin-bottom:1vh;font-family:var(--cFn),sans-serif;filter:drop-shadow(-5px -5px 5px var(--sCol))}select{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.5ch solid var(--bCol);filter:drop-shadow(-5px -5px 5px var(--sCol))}input[type=number]{background:var(--bCol);color:var(--dCol);border:.5ch solid var(--bCol);font-family:var(--cFn),sans-serif;width:3em}input[type=range]{-webkit-appearance:none;margin:-4px 0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:12vh;cursor:pointer;background:var(--bCol)}input[type=range]::-webkit-slider-thumb{filter:drop-shadow(-5px -5px 5px var(--sCol));height:10vh;width:10vh;background:var(--aCol);cursor:pointer;-webkit-appearance:none;margin-top:1vh}input[type=range]::-moz-range-track{width:100%;height:12vh;cursor:pointer;background:var(--bCol)}input[type=range]::-moz-range-thumb{filter:drop-shadow(-5px -5px 5px var(--sCol));height:10vh;width:10vh;background:var(--aCol);cursor:pointer;margin-top:1vh}input[type=range]::-ms-track{width:100%;height:12vh;cursor:pointer;background:transparent;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#var(--bCol)}input[type=range]::-ms-fill-upper{background:#var(--bCol)}input[type=range]::-ms-thumb{width:10vh;background:var(--aCol);cursor:pointer;height:10vh}</style>
<style id=holderjs-style type=text/css></style></head>
<body onload=Startup() class=__plain_text_READY__>
<span class=desc>Loading...</span>
)=====";

//body0 (svg defs)
const char PAGE_index2[] PROGMEM = R"=====(
<svg style=position:absolute;width:0;height:0;overflow:hidden version=1.1 xmlns=http://www.w3.org/2000/svg>
<defs>
//Linearicons.com/free
<symbol id=lnr-power-switch viewBox="0 0 1024 1024"><path d="M486.4 614.4c-14.138 0-25.6-11.461-25.6-25.6v-460.8c0-14.138 11.462-25.6 25.6-25.6s25.6 11.462 25.6 25.6v460.8c0 14.139-11.462 25.6-25.6 25.6z"></path><path d="M486.4 972.8c-102.57 0-199-39.944-271.53-112.47-72.528-72.528-112.47-168.96-112.47-271.53 0-84.395 26.859-164.478 77.674-231.594 49.15-64.915 118.979-113.394 196.624-136.501 13.55-4.034 27.805 3.683 31.838 17.234s-3.683 27.805-17.234 31.838c-139.955 41.654-237.702 172.84-237.702 319.022 0 183.506 149.294 332.8 332.8 332.8s332.8-149.294 332.8-332.8c0-146.187-97.75-277.374-237.71-319.024-13.552-4.034-21.267-18.288-17.234-31.838 4.032-13.552 18.29-21.267 31.837-17.235 77.646 23.106 147.48 71.582 196.632 136.499 50.816 67.115 77.675 147.202 77.675 231.598 0 102.57-39.942 199.002-112.47 271.53-72.528 72.526-168.96 112.47-271.53 112.47z"></path></symbol>
<symbol id=lnr-list viewBox="0 0 1024 1024"><path d="M998.4 819.2h-768c-14.138 0-25.6-11.461-25.6-25.6s11.462-25.6 25.6-25.6h768c14.139 0 25.6 11.461 25.6 25.6s-11.461 25.6-25.6 25.6z"></path><path d="M998.4 563.2h-768c-14.138 0-25.6-11.461-25.6-25.6s11.462-25.6 25.6-25.6h768c14.139 0 25.6 11.461 25.6 25.6s-11.461 25.6-25.6 25.6z"></path><path d="M998.4 307.2h-768c-14.138 0-25.6-11.462-25.6-25.6s11.462-25.6 25.6-25.6h768c14.139 0 25.6 11.462 25.6 25.6s-11.461 25.6-25.6 25.6z"></path><path d="M76.8 358.4c-42.347 0-76.8-34.453-76.8-76.8s34.453-76.8 76.8-76.8 76.8 34.453 76.8 76.8-34.453 76.8-76.8 76.8zM76.8 256c-14.115 0-25.6 11.485-25.6 25.6s11.485 25.6 25.6 25.6 25.6-11.485 25.6-25.6-11.485-25.6-25.6-25.6z"></path><path d="M76.8 614.4c-42.347 0-76.8-34.451-76.8-76.8 0-42.347 34.453-76.8 76.8-76.8s76.8 34.453 76.8 76.8c0 42.349-34.453 76.8-76.8 76.8zM76.8 512c-14.115 0-25.6 11.485-25.6 25.6s11.485 25.6 25.6 25.6 25.6-11.485 25.6-25.6-11.485-25.6-25.6-25.6z"></path><path d="M76.8 870.4c-42.347 0-76.8-34.451-76.8-76.8s34.453-76.8 76.8-76.8 76.8 34.451 76.8 76.8-34.453 76.8-76.8 76.8zM76.8 768c-14.115 0-25.6 11.485-25.6 25.6s11.485 25.6 25.6 25.6 25.6-11.485 25.6-25.6-11.485-25.6-25.6-25.6z"></path></symbol>
<symbol id=lnr-moon viewBox="0 0 1024 1024"><path d="M524.8 1024c-140.179 0-271.968-54.589-371.090-153.71s-153.71-230.91-153.71-371.090c0-115.242 37-224.010 106.997-314.546 33.528-43.365 73.674-81.155 119.325-112.323 46.050-31.44 96.664-55.424 150.437-71.285 9.819-2.901 20.424 0.336 26.957 8.222s7.738 18.906 3.064 28.016c-33.006 64.339-48.379 125.702-48.379 193.115 0 239.97 195.23 435.2 435.2 435.2 67.414 0 128.776-15.371 193.115-48.378 9.107-4.674 20.13-3.469 28.016 3.064s11.12 17.134 8.224 26.957c-15.861 53.773-39.845 104.387-71.286 150.435-31.166 45.65-68.958 85.797-112.323 119.325-90.536 69.998-199.306 106.997-314.546 106.997zM336.397 69.896c-172.741 73.982-285.197 240.059-285.197 429.304 0 261.144 212.456 473.6 473.6 473.6 189.234 0 355.314-112.475 429.304-285.197-52.406 19.598-105.373 29.197-160.504 29.197-129.922 0-252.067-50.594-343.936-142.462-91.869-91.87-142.464-214.016-142.464-343.938 0-55.131 9.6-108.098 29.197-160.504z"></path></symbol>
<symbol id=lnr-cog viewBox="0 0 1024 1024"><path d="M390.71 1008.755c-2.109 0-4.248-0.262-6.378-0.81-45.976-11.803-90.149-30.042-131.291-54.21-11.923-7.003-16.13-22.21-9.501-34.344 8.15-14.925 12.459-31.866 12.459-48.992 0-56.464-45.936-102.4-102.4-102.4-17.125 0-34.066 4.309-48.992 12.459-12.133 6.627-27.339 2.421-34.342-9.501-24.17-41.142-42.408-85.315-54.211-131.293-3.333-12.989 3.92-26.349 16.629-30.629 41.699-14.037 69.717-53.034 69.717-97.037s-28.018-83-69.718-97.040c-12.707-4.278-19.962-17.638-16.627-30.627 11.803-45.976 30.042-90.149 54.211-131.291 7.003-11.923 22.21-16.13 34.344-9.501 14.923 8.15 31.864 12.459 48.99 12.459 56.464 0 102.4-45.936 102.4-102.4 0-17.126-4.309-34.067-12.459-48.99-6.629-12.134-2.422-27.341 9.501-34.344 41.141-24.168 85.314-42.408 131.291-54.211 12.994-3.334 26.349 3.92 30.627 16.627 14.040 41.701 53.037 69.718 97.040 69.718s83-28.018 97.038-69.717c4.28-12.71 17.645-19.965 30.629-16.629 45.976 11.802 90.15 30.042 131.293 54.211 11.922 7.003 16.128 22.208 9.501 34.342-8.152 14.926-12.461 31.867-12.461 48.992 0 56.464 45.936 102.4 102.4 102.4 17.126 0 34.067-4.309 48.992-12.459 12.138-6.629 27.341-2.421 34.344 9.501 24.166 41.141 42.406 85.314 54.21 131.291 3.334 12.989-3.918 26.349-16.627 30.627-41.701 14.040-69.718 53.037-69.718 97.040s28.018 83 69.718 97.038c12.707 4.28 19.962 17.638 16.627 30.629-11.803 45.976-30.042 90.15-54.21 131.291-7.005 11.925-22.208 16.128-34.344 9.502-14.926-8.152-31.867-12.461-48.992-12.461-56.464 0-102.4 45.936-102.4 102.4 0 17.125 4.309 34.066 12.461 48.992 6.627 12.136 2.421 27.341-9.502 34.344-41.141 24.166-85.314 42.406-131.291 54.21-12.992 3.336-26.349-3.918-30.629-16.627-14.038-41.701-53.035-69.718-97.038-69.718s-83 28.018-97.040 69.718c-3.578 10.624-13.502 17.437-24.25 17.437zM512 870.4c57.715 0 109.693 32.138 135.917 82.029 26.637-8.218 52.507-18.875 77.299-31.846-5.541-16.077-8.416-33.075-8.416-50.182 0-84.696 68.904-153.6 153.6-153.6 17.107 0 34.106 2.875 50.181 8.418 12.971-24.792 23.63-50.662 31.846-77.299-49.89-26.226-82.027-78.203-82.027-135.918s32.138-109.691 82.029-135.918c-8.218-26.637-18.875-52.506-31.846-77.299-16.077 5.542-33.074 8.418-50.182 8.418-84.696 0-153.6-68.904-153.6-153.6 0-17.107 2.875-34.106 8.418-50.181-24.792-12.971-50.662-23.63-77.299-31.846-26.226 49.89-78.203 82.027-135.918 82.027s-109.691-32.138-135.917-82.027c-26.637 8.216-52.507 18.874-77.299 31.846 5.542 16.075 8.416 33.072 8.416 50.181 0 84.696-68.904 153.6-153.6 153.6-17.109 0-34.106-2.874-50.181-8.418-12.973 24.794-23.63 50.662-31.846 77.299 49.89 26.227 82.027 78.203 82.027 135.918s-32.138 109.693-82.027 135.917c8.216 26.637 18.875 52.507 31.846 77.299 16.075-5.541 33.074-8.416 50.181-8.416 84.696 0 153.6 68.904 153.6 153.6 0 17.109-2.875 34.106-8.418 50.181 24.794 12.971 50.662 23.63 77.299 31.846 26.227-49.89 78.203-82.027 135.918-82.027z"></path><path d="M512 665.6c-84.696 0-153.6-68.904-153.6-153.6s68.904-153.6 153.6-153.6 153.6 68.904 153.6 153.6-68.904 153.6-153.6 153.6zM512 409.6c-56.464 0-102.4 45.936-102.4 102.4s45.936 102.4 102.4 102.4c56.464 0 102.4-45.936 102.4-102.4s-45.936-102.4-102.4-102.4z"></path></symbol>
<symbol id=lnr-heart viewBox="0 0 1024 1024"><path d="M486.4 972.8c-4.283 0-8.566-1.074-12.434-3.222-4.808-2.67-119.088-66.624-235.122-171.376-68.643-61.97-123.467-125.363-162.944-188.418-50.365-80.443-75.901-160.715-75.901-238.584 0-148.218 120.582-268.8 268.8-268.8 50.173 0 103.462 18.805 150.051 52.952 27.251 19.973 50.442 44.043 67.549 69.606 17.107-25.565 40.299-49.634 67.55-69.606 46.589-34.147 99.878-52.952 150.050-52.952 148.218 0 268.8 120.582 268.8 268.8 0 77.869-25.538 158.141-75.901 238.584-39.478 63.054-94.301 126.446-162.944 188.418-116.034 104.754-230.314 168.706-235.122 171.376-3.867 2.149-8.15 3.222-12.434 3.222zM268.8 153.6c-119.986 0-217.6 97.614-217.6 217.6 0 155.624 120.302 297.077 221.224 388.338 90.131 81.504 181.44 138.658 213.976 158.042 32.536-19.384 123.845-76.538 213.976-158.042 100.922-91.261 221.224-232.714 221.224-388.338 0-119.986-97.616-217.6-217.6-217.6-87.187 0-171.856 71.725-193.314 136.096-3.485 10.453-13.267 17.504-24.286 17.504s-20.802-7.051-24.286-17.504c-21.456-64.371-106.125-136.096-193.314-136.096z"></path></symbol>
<symbol id=lnr-star viewBox="0 0 1024 1024"><path d="M793.598 972.8c-4.205 0-8.422-1.034-12.258-3.126l-269.341-146.912-269.341 146.912c-8.598 4.691-19.118 4.061-27.098-1.613-7.981-5.677-12.022-15.41-10.413-25.069l49.034-294.206-195.483-195.485c-6.781-6.781-9.203-16.782-6.277-25.914s10.712-15.862 20.17-17.438l294.341-49.058 122.17-244.339c4.336-8.674 13.2-14.152 22.898-14.152s18.562 5.478 22.898 14.152l122.17 244.339 294.341 49.058c9.459 1.576 17.243 8.307 20.17 17.438s0.504 19.133-6.277 25.914l-195.483 195.485 49.034 294.206c1.61 9.659-2.434 19.392-10.413 25.069-4.419 3.144-9.621 4.739-14.84 4.739zM512 768c4.219 0 8.437 1.042 12.259 3.126l235.445 128.426-42.557-255.344c-1.36-8.155 1.302-16.464 7.15-22.309l169.626-169.626-258.131-43.022c-8.080-1.346-15.027-6.477-18.69-13.803l-105.102-210.205-105.102 210.205c-3.664 7.326-10.61 12.458-18.69 13.803l-258.131 43.022 169.624 169.626c5.846 5.845 8.509 14.155 7.15 22.309l-42.557 255.344 235.446-128.426c3.821-2.085 8.040-3.126 12.259-3.126z"></path></symbol>
<symbol id=lnr-cross viewBox="0 0 1024 1024"><path d="M548.203 537.6l289.099-289.098c9.998-9.998 9.998-26.206 0-36.205-9.997-9.997-26.206-9.997-36.203 0l-289.099 289.099-289.098-289.099c-9.998-9.997-26.206-9.997-36.205 0-9.997 9.998-9.997 26.206 0 36.205l289.099 289.098-289.099 289.099c-9.997 9.997-9.997 26.206 0 36.203 5 4.998 11.55 7.498 18.102 7.498s13.102-2.499 18.102-7.499l289.098-289.098 289.099 289.099c4.998 4.998 11.549 7.498 18.101 7.498s13.102-2.499 18.101-7.499c9.998-9.997 9.998-26.206 0-36.203l-289.098-289.098z"></path></symbol>
<symbol id=lnr-arrow-left-circle viewBox="0 0 1024 1024"><path d="M142.462 193.664c91.869-91.869 214.016-142.464 343.938-142.464s252.067 50.595 343.936 142.464 142.464 214.014 142.464 343.936-50.595 252.069-142.464 343.938-214.014 142.462-343.936 142.462-252.069-50.594-343.938-142.462-142.462-214.016-142.462-343.938 50.594-252.067 142.462-343.936zM486.4 972.8c239.97 0 435.2-195.23 435.2-435.2s-195.23-435.2-435.2-435.2c-239.97 0-435.2 195.23-435.2 435.2s195.23 435.2 435.2 435.2z"></path><path d="M186.701 519.501l204.8-204.8c9.995-9.998 26.206-9.998 36.203 0 9.998 9.997 9.998 26.206 0 36.203l-161.101 161.096h526.997c14.138 0 25.6 11.461 25.6 25.6s-11.462 25.6-25.6 25.6h-526.997l161.096 161.101c9.998 9.995 9.998 26.206 0 36.203-4.997 4.997-11.547 7.496-18.099 7.496s-13.102-2.499-18.099-7.501l-204.8-204.8c-10-9.997-10-26.202 0-36.198z"></path></symbol>
<symbol id=lnr-arrow-right-circle viewBox="0 0 1024 1024"><path d="M830.338 193.664c-91.869-91.869-214.016-142.464-343.938-142.464s-252.067 50.595-343.936 142.464-142.464 214.014-142.464 343.936 50.595 252.069 142.464 343.938 214.014 142.462 343.936 142.462 252.069-50.594 343.938-142.462 142.462-214.016 142.462-343.938-50.594-252.067-142.462-343.936zM486.4 972.8c-239.97 0-435.2-195.23-435.2-435.2s195.23-435.2 435.2-435.2c239.97 0 435.2 195.23 435.2 435.2s-195.23 435.2-435.2 435.2z"></path><path d="M786.099 519.501l-204.8-204.8c-9.995-9.998-26.206-9.998-36.203 0-9.998 9.997-9.998 26.206 0 36.203l161.101 161.096h-526.997c-14.138 0-25.6 11.461-25.6 25.6s11.462 25.6 25.6 25.6h526.997l-161.096 161.101c-9.998 9.995-9.998 26.206 0 36.203 4.997 4.997 11.547 7.496 18.099 7.496s13.102-2.499 18.099-7.501l204.8-204.8c10-9.997 10-26.202 0-36.198z"></path></symbol>
<symbol id=lnr-rocket viewBox="0 0 1024 1024"><path d="M691.2 460.8c-70.579 0-128-57.421-128-128s57.421-128 128-128 128 57.421 128 128-57.421 128-128 128zM691.2 256c-42.347 0-76.8 34.453-76.8 76.8s34.453 76.8 76.8 76.8 76.8-34.453 76.8-76.8-34.453-76.8-76.8-76.8z"></path><path d="M25.603 1024c-6.675 0-13.219-2.613-18.106-7.499-7.034-7.034-9.355-17.502-5.957-26.85 78.781-216.648 161.613-326.499 246.195-326.499 27.883 0 53.979 11.96 77.566 35.546 37.283 37.283 38.611 74.394 33.162 98.96-17.125 77.187-126.171 152.822-324.115 224.802-2.853 1.038-5.813 1.541-8.746 1.541zM247.736 714.354c-25.354 0-55.19 22.214-86.282 64.237-30.578 41.33-61.274 100.205-91.525 175.477 68.352-27.478 123.302-55.379 163.806-83.205 54.648-37.542 70.808-66.562 74.742-84.294 3.944-17.779-2.395-34.682-19.382-51.667-13.826-13.826-27.354-20.547-41.36-20.547z"></path><path d="M998.4 0c-132.848 0-251.256 22.534-351.939 66.981-82.997 36.638-154.075 88.075-211.258 152.882-10.674 12.098-20.552 24.334-29.691 36.586-44.142 2.942-89.275 20.47-134.362 52.221-38.13 26.851-76.459 64.014-113.923 110.458-62.965 78.054-101.706 154.987-103.325 158.226-5.605 11.211-2.25 24.814 7.904 32.166 4.501 3.258 9.758 4.856 14.992 4.856 6.573 0 13.109-2.52 18.064-7.434 0.243-0.24 24.714-24.299 66.469-47.926 34.41-19.474 87.461-42.336 151.613-46.384 16.219 41.541 62.662 91.181 84.954 113.47 22.291 22.291 71.931 68.734 113.472 84.955-4.046 64.152-26.91 117.202-46.382 151.611-23.629 41.757-47.686 66.227-47.89 66.432-8.878 8.878-10.006 22.885-2.666 33.070 4.952 6.87 12.77 10.634 20.782 10.634 3.867 0 7.779-0.877 11.434-2.704 3.237-1.619 80.17-40.36 158.226-103.325 46.443-37.464 83.606-75.794 110.458-113.922 31.75-45.088 49.278-90.221 52.221-134.363 12.251-9.139 24.49-19.019 36.586-29.693 64.806-57.181 116.243-128.259 152.883-211.258 44.443-100.682 66.979-219.091 66.979-351.939v-25.6h-25.6zM159.102 502.187c48.797-70.8 123.384-158.595 207.446-186.232-33.222 58.203-50.422 111.691-56.611 145.555-59.323 3.626-110.467 20.89-150.835 40.677zM521.87 864.781c19.762-40.35 36.995-91.453 40.619-150.718 33.859-6.187 87.336-23.384 145.528-56.597-27.658 83.92-115.381 158.49-186.147 207.315zM770.262 550.405c-106.48 93.952-216.794 115.195-232.662 115.195-0.102 0-10.581-0.23-38.867-20.136-19.728-13.883-42.682-33.618-64.63-55.566-21.95-21.95-41.683-44.902-55.566-64.632-19.906-28.285-20.136-38.763-20.136-38.866 0-15.869 21.243-126.182 115.197-232.662 112.416-127.406 284.533-197.059 498.894-202.227-5.17 214.358-74.822 386.477-202.229 498.894z"></path></symbol>
<symbol id=lnr-sync viewBox="0 0 1024 1024"><path id=path1 d="M1016.501 442.698c-9.997-9.997-26.206-9.997-36.203 0l-58.832 58.832c-2.63-105.486-44.947-204.27-119.835-279.16-77.362-77.365-180.222-119.97-289.63-119.97-152.28 0-291.122 83.699-362.342 218.435-6.606 12.499-1.83 27.989 10.669 34.597 12.498 6.606 27.989 1.83 34.597-10.669 62.33-117.914 183.826-191.163 317.077-191.163 194.014 0 352.501 154.966 358.224 347.619l-58.522-58.522c-9.997-9.997-26.206-9.997-36.203 0-9.998 9.998-9.998 26.206 0 36.205l102.4 102.4c4.998 4.998 11.549 7.498 18.101 7.498s13.102-2.499 18.101-7.499l102.4-102.4c9.998-9.997 9.998-26.205 0-36.203z"></path><path id=path2 d="M863.674 668.566c-12.502-6.603-27.99-1.832-34.597 10.669-62.328 117.915-183.826 191.165-317.077 191.165-194.016 0-352.502-154.966-358.224-347.621l58.522 58.522c5 5 11.55 7.499 18.102 7.499s13.102-2.499 18.102-7.499c9.997-9.997 9.997-26.206 0-36.203l-102.4-102.4c-9.998-9.997-26.206-9.997-36.205 0l-102.4 102.4c-9.997 9.997-9.997 26.206 0 36.203s26.206 9.997 36.205 0l58.83-58.832c2.63 105.488 44.946 204.272 119.835 279.162 77.365 77.363 180.224 119.97 289.632 119.97 152.28 0 291.12-83.699 362.342-218.435 6.608-12.501 1.829-27.99-10.669-34.598z"></path></symbol>
<symbol id=lnr-checkmark-circle viewBox="0 0 1024 1024"><path d="M486.4 1024c-129.922 0-252.067-50.594-343.936-142.464s-142.464-214.014-142.464-343.936c0-129.923 50.595-252.067 142.464-343.936s214.013-142.464 343.936-142.464c129.922 0 252.067 50.595 343.936 142.464s142.464 214.014 142.464 343.936-50.594 252.067-142.464 343.936c-91.869 91.87-214.014 142.464-343.936 142.464zM486.4 102.4c-239.97 0-435.2 195.23-435.2 435.2s195.23 435.2 435.2 435.2 435.2-195.23 435.2-435.2-195.23-435.2-435.2-435.2z"></path><path d="M384 742.4c-6.552 0-13.102-2.499-18.102-7.499l-153.6-153.6c-9.997-9.997-9.997-26.206 0-36.203 9.998-9.997 26.206-9.997 36.205 0l135.498 135.498 340.299-340.298c9.997-9.997 26.206-9.997 36.203 0 9.998 9.998 9.998 26.206 0 36.205l-358.4 358.4c-5 4.998-11.55 7.498-18.102 7.498z"></path></symbol>
<symbol id=lnr-arrow-down-circle viewBox="0 0 1024 1024"><path d="M830.336 881.538c91.869-91.869 142.464-214.016 142.464-343.938s-50.595-252.067-142.464-343.936-214.014-142.464-343.936-142.464-252.069 50.595-343.938 142.464-142.462 214.014-142.462 343.936 50.594 252.069 142.462 343.938 214.016 142.462 343.938 142.462 252.067-50.594 343.936-142.462zM51.2 537.6c0-239.97 195.23-435.2 435.2-435.2s435.2 195.23 435.2 435.2c0 239.97-195.23 435.2-435.2 435.2s-435.2-195.23-435.2-435.2z"></path><path d="M504.499 837.299l204.8-204.8c9.998-9.995 9.998-26.206 0-36.203-9.997-9.998-26.206-9.998-36.203 0l-161.096 161.101v-526.997c0-14.138-11.461-25.6-25.6-25.6s-25.6 11.462-25.6 25.6v526.997l-161.101-161.096c-9.995-9.998-26.206-9.998-36.203 0-4.997 4.997-7.496 11.547-7.496 18.099s2.499 13.102 7.501 18.099l204.8 204.8c9.997 10 26.202 10 36.198 0z"></path></symbol>
</defs>
</svg>
)=====";

//body1 (html)
const char PAGE_index3[] PROGMEM = R"=====(
<div id=tbB class=tool_box>
<svg id=tgb onclick=TgT()><use xlink:href=#lnr-power-switch></use></svg>
<svg id=mdb onclick=TgHSB()><use xlink:href=#lnr-list></use></svg>
<svg id=psb onclick=CV(2)><use xlink:href=#lnr-heart></use></svg>
<svg id=fxb onclick=CV(3)><use xlink:href=#lnr-star></use></svg>
<svg id=nlb onclick=CV(4)><use xlink:href=#lnr-moon></use></svg>
<svg id=nsb onclick=TgN()><use xlink:href=#lnr-sync></use></svg>
<svg id=stb onclick=TgS()><use xlink:href=#lnr-cog></use></svg>
</div>
<div id=cdB class=ctrl_box>
<form id=form_c name=Cf>
<br>
<div id=slA class=sl>
<input type=range title=Brightness class=sds name=SA value=0 min=0 max=255 step=1 onchange=GC()></div>
<div id=srgb>
<div id=slR class=sl>
<input type=range title="Red Value" class=sds name=SR value=0 min=0 max=255 step=1 onchange=GC()></div>
<div id=slG class=sl>
<input type=range title="Green Value" class=sds name=SG value=0 min=0 max=255 step=1 onchange=GC()></div>
<div id=slB class=sl>
<input type=range title="Blue Value" class=sds name=SB value=0 min=0 max=255 step=1 onchange=GC()></div></div>
<div id=shs>
<div id=slH class=sl>
<input type=range title=Hue class=sds name=SH value=0 min=0 max=1 step=0.025 onchange=GetRGB()></div>
<div id=slS class=sl>
<input type=range title=Saturation class=sds name=SS value=0 min=0 max=1 step=0.025 onchange=GetRGB()></div></div>
<div id=slW class=sl>
<input type=range title="White Value" class=sds name=SW value=0 min=0 max=255 step=1 onchange=GC()></div>
<div id=tlX class=tools>
Effect Panel<br><br>
<svg id=fmr onclick=SwFX(-99)><use xlink:href=#lnr-cross></use></svg>
<svg id=for onclick=SwFX(-1)><use xlink:href=#lnr-arrow-left-circle></use></svg>
<svg id=fmf onclick=SwFX(1)><use xlink:href=#lnr-arrow-right-circle></use></svg>
<svg id=fof onclick=SwFX(99)><use xlink:href=#lnr-rocket></use></svg><br><br>
<select name=TX onchange=GX()>
<option value=0 selected>Solid (0)</option>
<option value=1>Blink (1)</option>
<option value=2>Breath (2)</option>
<option value=3>Wipe (3)</option>
<option value=4>Wipe Random (4)</option>
<option value=5>Color R (5)</option>
<option value=6>Easter (6)</option>
<option value=7>Dynamic (7)</option>
<option value=8>Colorloop (8)</option>
<option value=9>Rainbow (9)</option>
<option value=10>Scan (10)</option>
<option value=11>Scan x2 (11)</option>
<option value=12>Fade (12)</option>
<option value=13>Chase (13)</option>
<option value=14>Chase Cl (14)</option>
<option value=15>Running (15)</option>
<option value=16>Twinkle (16)</option>
<option value=17>Twinkle R (17)</option>
<option value=18>Twinkle Fade (18)</option>
<option value=19>Twinkle RF (19)</option>
<option value=20>Sparkle (20)</option>
<option value=21>Sparkle Inv (21)</option>
<option value=22>Sparkle Inv+ (22)</option>
<option value=23>Strobe (23)</option>
<option value=24>Strobe Cl (24)</option>
<option value=25>Strobe + (25)</option>
<option value=26>Blink Cl (26)</option>
<option value=27>Android (27)</option>
<option value=28>Chase (28)</option>
<option value=29>Chase R (29)</option>
<option value=30>Chase Rainbow (30)</option>
<option value=31>Chase Flash (31)</option>
<option value=32>Chase RF (32)</option>
<option value=33>Chase Cl Inv (33)</option>
<option value=34>Colorful (34)</option>
<option value=35>Traffic Light (35)</option>
<option value=36>Sweep R(36)</option>
<option value=37>Running 2 (37)</option>
<option value=38>Red/Blue (38)</option>
<option value=39>Running R (39)</option>
<option value=40>Scanner (40)</option>
<option value=41>Lighthouse (41)</option>
<option value=42>Fireworks (42)</option>
<option value=43>Fireworks R (43)</option>
<option value=44>Christmas (44)</option>
<option value=45>Fire Flicker (45)</option>
<option value=46>Gradient (46)</option>
<option value=47>Loading (47)</option>
<option value=48>Wipe IO (48)</option>
<option value=49>Wipe II (49)</option>
<option value=50>Wipe OO (50)</option>
<option value=51>Wipe OI (51)</option>
<option value=52>Circus (52)</option>
<option value=53>Custom Chase (53)</option>
<option value=54>CC Colorloop (54)</option>
<option value=55>CC Rainbow (55)</option>
<option value=56>CC Blink (56)</option>
<option value=57>CC Random (57)</option>
</select><br><br>
Set secondary color to
<button type=button onclick=CS(0)>White</button>
<button type=button onclick=CS(1)>Black</button>
<button type=button onclick=CS(2)>Random</button>
<button type=button onclick=CS(3)>Primary</button>
<button type=button onclick=CS(4)>Swap P/S</button>
or <button type=button onclick=CS(5)>Set Primary to Random</button>
<div id=ccX>
<br>Custom Theater Chase<br>
using <input id=ccP name=PF type=number value=2 min=0 max=255 step=1 onchange=GetCC()> primary and
<input id=ccS name=SF type=number value=4 min=0 max=255 step=1 onchange=GetCC()> secondary color LEDs,<br>
doing <input id=ccH name=HF type=number value=1 min=0 max=255 step=1 onchange=GetCC()> steps per tick,
from <input type=checkbox onchange=GetCC() name=SC> start and <input type=checkbox onchange=GetCC() name=EC> end.
</div>
<div id=slX class=sl>
<input type=range title="Effect Speed" class=sds name=SX value=0 min=0 max=255 step=1 onchange=GX()></div>
<div id=slI class=sl>
<input type=range title="Effect Intensity" class=sds name=IX value=0 min=0 max=255 step=1 onchange=GX()></div>
</div>
<div id=tlP class=tools>
Favorite Presets<br><br>
<svg id=psl onclick=PSIO(false)><use xlink:href=#lnr-checkmark-circle></use></svg>
<svg id=psp onclick=SwitchPS(-1)><use xlink:href=#lnr-arrow-left-circle></use></svg>
<svg id=psn onclick=SwitchPS(1)><use xlink:href=#lnr-arrow-right-circle></use></svg>
<svg id=pss onclick=PSIO(true)><use xlink:href=#lnr-arrow-down-circle></use></svg>
<br><input id=psI name=FF type=number value=1 min=1 max=25 step=1><br><br>
Click checkmark to apply <input type="checkbox" checked="true" name="BC"> brightness, <input type="checkbox" checked="true" name="CC"> color and <input type="checkbox" checked="true" name="FC"> effects.<br><br>
Cycle through presets <input id="cy1" name="P1" type="number" value="1" min="1" max="25" step="1"> to <input id="cy2" name="P2" type="number" value="5" min="1" max="25" step="1">, keep each for <input id="cyT" name="PT" type="number" value="1250" min="50" max="65501" step="1">ms: <input type="checkbox" name="CY" onclick="uCY()"><br><br>
<button type="button" onclick="R()">Apply boot config</button><br></div>
<div id=tlN class=tools>
Timed Light<br><br>
<svg id=ntb onclick=TgNl()><use xlink:href=#lnr-power-switch></use></svg><br><br>
Gradually dim down <input type=checkbox checked=true name=NC><br>
1st slider sets duration (1-255min), 2nd sets target brightness.
<div id=slN class=sl>
<input type=range title=Duration class=sds name=SN value=60 min=1 max=255 step=1></div>
<div id=slT class=sl>
<input type=range title="Target Brightness" class=sds name=ST value=0 min=0 max=255 step=1></div>
</div>
<br>
</form>
</div>
<iframe id=stf src=about:blank></iframe>
</body>
</html>
)=====";
