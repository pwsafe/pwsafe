// Created on 2007-06-17
package org.pwsafe.passwordsafeswt.preference;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.jface.preference.PreferenceConverter;
import org.eclipse.jface.preference.PreferenceStore;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.ShellAdapter;
import org.eclipse.swt.events.ShellEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.TableColumn;
import org.pwsafe.passwordsafeswt.util.UserPreferences;

/**
 * Widget related preferences
 * 
 * @author shamilbi
 */
public class WidgetPreferences
{
    static final Log log = LogFactory.getLog(WidgetPreferences.class);

    private static final String PREFS_FILENAME = "widget.properties";

    private static final String FULL_PATH = System.getProperty("user.home")
            + File.separator + UserPreferences.PROPS_DIR + File.separator
            + PREFS_FILENAME;

    public static final PreferenceStore ps = new PreferenceStore(FULL_PATH);

    static
    {
        try
        {
            log.info("loading preferences ..");
            ps.load();
        } catch (FileNotFoundException e)
        {
            // ignore
        } catch (IOException e)
        {
            log.warn("Error while loading preferences", e);
        }
    }

    public static void tuneShell(final Shell shell, final Class clazz)
    {
        WidgetPreferences.restoreLocation(shell, clazz);
        shell.addShellListener(new ShellAdapter()
        {
            public void shellClosed(ShellEvent e)
            {
                save();
            }
        });
        
        shell.addDisposeListener(new DisposeListener()
        {
            public void widgetDisposed(DisposeEvent e)
            {
                saveLocation(shell, clazz);
            }
        });
    }
    
    public static void tuneTableColumn(final TableColumn column, final String id)
    {
        final Rectangle rectangle = PreferenceConverter.getRectangle(ps, id);
        if (rectangle.width > 0)
        {
            column.setWidth(rectangle.width);
        }

        column.addDisposeListener(new DisposeListener()
        {
            public void widgetDisposed(DisposeEvent e)
            {
                log.debug("saving widget size, id=" + id);
                PreferenceConverter.setValue(ps, id, new Rectangle(0, 0, column
                        .getWidth(), 0));
            }
        });

    }

    public static void tuneTableColumn(final TableColumn column, Class clazz,
            final String id)
    {
        tuneTableColumn(column, clazz.getName() + "/" + id);
    }

    public static void save()
    {
        log.info("saving preferences ..");
        try
        {
            ps.save();
        } catch (IOException e)
        {
            log.warn("Error while saving preferences", e);
        }
    }

    public static void restoreLocation(Shell shell, String id)
    {
        final Rectangle rectangle = PreferenceConverter.getRectangle(ps, id);
        if (rectangle.x != 0 || rectangle.y != 0)
        {
            shell.setLocation(rectangle.x, rectangle.y);
        }

        if (rectangle.width > 0 && rectangle.height > 0)
        {
            shell.setSize(rectangle.width, rectangle.height);
        }
    }

    public static void restoreLocation(Shell shell, Class clazz)
    {
        restoreLocation(shell, clazz.getName() + "/window");
    }

    public static void saveLocation(Shell shell, String id)
    {
        log.debug("saving window location, id=" + id);
        final Point location = shell.getLocation();
        final Point size = shell.getSize();
        final Rectangle rectangle = new Rectangle(location.x, location.y, size.x, size.y);
        PreferenceConverter.setValue(ps, id, rectangle);
    }

    public static void saveLocation(Shell shell, Class clazz)
    {
        saveLocation(shell, clazz.getName() + "/window");
    }

}
