var main = function(){
 
$('.article').attr('title','closed');


 $('.article').click(function (){

$('.article').removeClass('current');


 $('.description').hide();
 
 $(this).addClass('current');
 $(this).children('.description').show();

 });
 
 
 $(document).keypress(function(event){
   
    var currentArticle
  /*111 is o key*/
   if(event.which === 111){
      currentArticle = $('.current');
       if(currentArticle.attr('title') == "opened"){
       		 currentArticle.children('.description').hide();
       		 currentArticle.attr('title','closed');

       }else if(currentArticle.attr('title') == "closed"){
       		currentArticle.children('.description').show();
       		currentArticle.attr('title','opened');
       }
     
     
      
   // 110 is n key
   }else if(event.which === 110){
        currentArticle = $('.current');
       var nextArticle = currentArticle.next();
       currentArticle.removeClass('current');
       nextArticle.addClass('current');
   }
 
 });

};

$(document).ready(main);
