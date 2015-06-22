var dirty = true;
function clearOutput()
{
	if (dirty)
	{
		document.auto.output.value = "Click the ENCODE button below\nto convert your email addresses\ninto encoded links.";
		dirty = false;
	}
}
function encode(addr)
{
	var name,dom;
	addr = addr.replace(/\-AT\-/gi,'@');
	i = addr.indexOf('@');
	name = protectQuotes(addr.substring(0,i));
	dom = protectQuotes(addr.substring(i+1));
	var tl = -1;
	for (i=0; i<=topDom_; i++) {
		if (tld_[i]!=null && dom.substring(dom.length-tld_[i].length).toLowerCase()==tld_[i]) {
			dom = dom.substring(0,dom.length-tld_[i].length-1);
			tl = i;
			break;
		}
        }

	if (tl<0) {
		tl = -2;
	}

	dom = swapper(dom).replace(/\./g,'?');
        name = swapper(name);

	return '"'+name+'","'+dom+'",'+tl+',""';
}
function encodeLink(addr)
{
	addr = trim(addr);
	var i = addr.indexOf('<');
	var s = 'mail';
	if (i>=0 && addr.charAt(addr.length-1)=='>')
	{
		var display = trim(trimQuotes(trim(addr.substring(0,i))));
		display = protectQuotes(display);
		addr = trim(addr.substring(i+1, addr.length-1));
		s+= '2('+encode(addr)+',"'+display+'"';
	}
	else
		s+= '('+encode(addr);
	s+= ')';
	return "<script>"+s+"</script>";
}
function encodeFreeHtml(line)
{
	var s = line;
	var j = s.indexOf("<a href");
	var s1,addr,display,s2;
	if (j>=0)
	{
		s1 = s.substring(0, j);
		j+= 7;
		while (s.charAt(j)==" " || s.charAt(j)=="\t" || s.charAt(j)=="=")
			j++;
		var c = s.charAt(j);
		if (c=='\'' || c=='\"')
		{
			var j2 = s.indexOf(c,j+1);
			if (j2>0)
			{
				addr = trim(s.substring(j+1,j2));
				j = addr.indexOf(":");
				if (j>0 && addr.toLowerCase().substring(0,6)=="mailto")
				{
					addr = addr.substring(j+1);
					j = j2+1;
					while (s.charAt(j)==" " || s.charAt(j)=="\t" || s.charAt(j)==">")
						j++;
					j2 = s.indexOf("</a>", j);
					if (j2>0)
					{
						display = s.substring(j,j2);
						s2 = s.substring(j2+4);
						s = s1+"<script>mail2(";
						s+= encode(addr)+',"'+protectQuotes(display)+'"';
						s+= ")</script>"+s2;
					}
				}
			}
		}
	}
	return s;
}
function encodeLinks()
{
	var s = document.auto.input.value;
	var lines = new Array();
	var i,j,j2,k;
	var line;
	i = 0;
	while (s.length>0)
	{
		j = s.indexOf("\r");
		j2 = s.indexOf("\n");
		if (j<0)
			j = j2;
		if (j2<0)
			j2 = j;
		if (j<0)
		{
			j = s.length;
			j2 = j-1;
		}
		line = trim(s.substring(0, j));
		if (line.length>0)
			lines[i++] = line;
		s = s.substring(j2+1);
	}
	s = "";
	for (i=0; i<lines.length; i++)
	{
		j = lines[i].indexOf("<a href");
		if (j>=0)
			s+= encodeFreeHtml(lines[i])+"\n";
		else
			s+= encodeLink(lines[i])+"\n";
	}
	document.auto.output.value = s;
	dirty = true;
}
function trim(s)
{
	while (s.length>0 && (s.charAt(0)==' ' || s.charAt(0)=='\t'))
		s = s.substring(1);
	while (s.length>0 && (s.charAt(s.length-1)==' ' || s.charAt(s.length-1)=='\t'))
		s = s.substring(0, s.length-1);
	return s;
}
function trimQuotes(s)
{
	while (s.length>0 && (s.charAt(0)=='"' || s.charAt(0)=='\''))
		s = s.substring(1);
	while (s.length>0 && (s.charAt(s.length-1)=='"' || s.charAt(s.length-1)=='\''))
		s = s.substring(0, s.length-1);
	return s;
}
function protectQuotes(s)
{
	return s.replace(/"/g, '\'');
}
