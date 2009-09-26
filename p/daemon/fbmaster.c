mixed names = ([]);

create()
{
    seteuid(getuid());
    restore_object("/p/daemon/save/fbmaster");
}

online()
{
    return "sommercamp";
}

markDone(object pl, string theme)
{
    theme = lower_case(theme);
    if(!names[theme]) names[theme] = ({});
    names[theme] += ({sprintf("%O", pl)});
    save_object("/p/daemon/save/fbmaster");
}

        

done(object pl, string theme)
{
    theme = lower_case(theme);
    if(sizeof(names) && names[theme])
        return member(names[theme], sprintf("%O", pl)) != -1;
}

undo(object pl, string theme)
{
    theme = lower_case(theme);
    if(sizeof(names) && names[theme])
        names[theme] -= ({sprintf("%O", pl)});
    save_object("/p/daemon/save/fbmaster");
}

SaveAnswer(mixed pl, mixed answer, string theme)
{
    mixed keys;
    theme = lower_case(implode(old_explode(theme, "\n"), ""));
    write_file("/log/answers/"+theme,  
               sprintf("%O#%s\n", pl, implode(answer, "#")));
    if(!names[theme]) names[theme] = ({});
    names[theme] += ({ sprintf("%O", pl) });
    save_object("/p/daemon/save/fbmaster");
}
