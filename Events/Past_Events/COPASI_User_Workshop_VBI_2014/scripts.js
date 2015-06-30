
jQuery(document).ready(function() {

    /*
        Background slideshow
    */
    $('.slideshow').backstretch([
      "1.jpg"
    , "2.jpg"
    , "3.jpg"
    , "4.jpg"
    , "5.jpg"
    ], {duration: 3000, fade: 750});

    /*
        Countdown initializer
    */
    $('.timer').countdown('2014/07/08', function(event) {
  		$(this).html(event.strftime('' 
       + '<div class="weeks-wrapper"><span class="weeks">%w</span> <br>	weeks</div> '
       + '<div class="days-wrapper"><span class="days">%d</span>  <br> days</div> '
       + '<div class="hours-wrapper"><span class="hours">%H</span>  <br> hours</div> '
       + '<div class="minutes-wrapper"><span class="minutes">%M</span>  <br> minutes</div> '
       + '<div class="seconds-wrapper"><span class="seconds">%S</span>  <br> seconds</div>'));
   });

    /*
        Tooltips
    */
    $('.social a.facebook').tooltip();
    $('.social a.twitter').tooltip();
    $('.social a.dribbble').tooltip();
    $('.social a.googleplus').tooltip();
    $('.social a.pinterest').tooltip();
    $('.social a.flickr').tooltip();

    /*
        Subscription form
    */
    $('.success-message').hide();
    $('.error-message').hide();

    $('.subscribe form').submit(function() {
        var postdata = $('.subscribe form').serialize();
        $.ajax({
            type: 'POST',
            url: 'assets/sendmail.php',
            data: postdata,
            dataType: 'json',
            success: function(json) {
                if(json.valid == 0) {
                    $('.success-message').hide();
                    $('.error-message').hide();
                    $('.error-message').html(json.message);
                    $('.error-message').fadeIn();
                }
                else {
                    $('.error-message').hide();
                    $('.success-message').hide();
                    $('.subscribe form').hide();
                    $('.success-message').html(json.message);
                    $('.success-message').fadeIn();
                }
            }
        });
        return false;
    });

});

