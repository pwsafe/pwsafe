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
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.TableColumn;
import org.pwsafe.passwordsafeswt.util.UserPreferences;

/**
 * Widget related preferences
 * 
 * @author shamilbi
 */
public class WidgetPreferences
{
    private static final Log log = LogFactory.getLog(WidgetPreferences.class);

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

}
