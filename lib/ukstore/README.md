# uk_store

uk_store provides Unikraft libraries with a generic API of getters and setters.
The API defines a producer / consumer model that implements pull-oriented getters and push-oriented setters.
Under that model, producer libraries define and export a set of entries, each one of which consists of a single value.
Consumer libraries retrieve and / or update entries using their associated getters and setter functions.

## Dynamic API

The dynamic API organizes exported entries of each library hierarchically under objects, as `library/object/entry[value]`.
A library can export multiple objects, and each object can export multiple entries.
Dynamic objects and entries are created at run time.

The dynamic API is useful for libraries that handle multiple instances of a given entity, such as a device driver handling multiple devices of the same type.

Producer libraries create objects using `uk_store_create_object()`.
This function initializes the new object along with a set of entries, each one of which is associated with a getter and / or a setter function.

The producer notifies consumers about the creation and release of new objects using `UK_STORE_EVENT_CREATE_OBJECT` / `UK_STORE_EVENT_RELEASE_OBJECT`.
Upon receiving the event, the consumers can enumerate the object's entries and access it's values.
If a getter function is defined for a given entry, consumers can retrieve it's value by calling `uk_store_get_value()`.
If a setter function is defined for a given entry, consumers can use `uk_store_set_value()`.

The following examples describes a simple case of a fictional device driver library, `uk_mydevice`, that uses the dynamic API to export a single metric, `irq_count`.
The library exports each device instance as a separate uk_store object.

### Example

#### Device initialization: Producer
```c
/* Entry ID for irq_count (arbitrary) */
#define UK_MYDEVICE_STATS_IRQ_COUNT   0xb0b0

/* This is the getter function for the irq_count entry */
static int get_irq_count(void *cookie, __u64 *out)
{
    /* We set the cookie to this device's instance when we
     * create the uk_store object (see below)
     */
    struct uk_mydevice *dev = (struct uk_mydevice *)cookie;

    /* The device somehow provides this metric */
    *out = dev->irq_count;

    return 0;
}

/* Define an array of entries for the stats object */
static const struct uk_store_entry *entries[] = {
    /* Registers an entry with a getter function. For more info on usage see stats.h */
    UK_STORE_ENTRY(UK_MYDEVICE_STATS_IRQ_COUNT, "irq_count", u64, get_irq_count, NULL, NULL),
    /* ... more entries ... */
    NULL /* terminates the array */
}

/* Called by the driver once a new device is probed */
int uk_mydevice_init_metrics(struct uk_mydevice *dev)
{
    int ret;
    struct uk_store_object *obj;
    struct uk_store_entry *entry;

    /* Create an object to export the stats of the new device. Also initializes the
     * refcount. By convention this library uses the new device's id and name that
     * are assumed to be initialized by the driver. Each library can establish its
     * own convention.
     *
     * We also pass the device in the cookie parameter so that the getter function
     * gets a referece to the device.
     *
     * For more info on usage see store.h
     */
    ret = uk_store_create_object(dev->id, dev->name, , entries, (void *)dev);
    if (unlikely(ret))
        return ret;

    /* Notify consumers */
    event_data = () {
        .library_id = UK_MYDEVICE_LIB_ID,
        .object_id = obj->id,
        .owner = NULL
    };
    uk_raise_event(UK_STORE_EVENT_CREATE_OBJECT, &event_data);

    return 0;
}
```

#### Device initialization: Consumer

```c
/* This is the handler for UK_STORE_EVENT_CREATE_OBJECT */
static int handle_create_object(void *arg)
{
    struct uk_store_event_data *data = (struct uk_store_event_data *)arg;

    /* Is this coming from our library? */
    if (data->library_id != UK_MYDEVICE_LIB_ID)
        return UK_EVENT_NOT_HANDLED; /* continue to next handler */

    /* Increments the refcount */
    obj = uk_store_acquire_object(data->library_id, data->object_id);
    if (unlikely(!obj))
        return UK_EVENT_HANDLED; /* The object was already released */

    /* At this point we hold a valid pointer of the object. It won't be
     * freed until we decrement the refcount.
     */
    entry = uk_store_get_entry(UK_MYDEVICE_LIB_ID, obj->id, UK_MYDEV_METRICS_IRQ_COUNT);

    /* Create /sys/mydevice/irq_count and associate this entry with that file.
     * Call `uk_store_get_value()` every time the user reads from /sys/mydevice/irq_count.
     */

    return UK_EVENT_HANDLED;
}

/* Registers handler */
UK_EVENT_HANDLER_(UK_STORE_EVENT_CREATE_OBJECT, handle_create_object);
```

#### Device Termination: Producer
```c
/* Called by the driver on device termination */
int uk_mydevice_teardown_metrics(struct uk_mydevice *dev)
{
    struct uk_store_object *obj;

    /* Is this coming from our library? */
    if (data->library_id != UK_MYDEVICE_LIB_ID)
        return UK_EVENT_NOT_HANDLED; /* continue to next handler */

    /* this is looked up via library-specific means */
    obj = get_obj_by_device(dev);

    /* Notify consumers */
    event_data = () {
        .library_id = UK_MYDEVICE_LIB_ID,
        .object_id = obj->id,
        .owner = NULL
    };
    uk_raise_event(UK_STORE_EVENT_RELEASE_OBJECT, &event_data);

    /* Decrement the refcount. The object will be freed as soon as
     * all consumers decrement the refcount.
     */
    uk_store_release_object(UK_MYDEVICE_LIB_ID, obj);

    return 0;
}
```

#### Device Termination: Consumer

```c
/* This is the handler for UK_STORE_EVENT_RELEASE_OBJECT */
static int handle_release_object(struct uk_event_data *data)
{
    struct uk_store_event_data *data = (struct uk_store_event_data *)arg;

    /* Use library-specific means to stop pulling values
     * and remove /sys/mydevice/irq_count.
     */

    /* Decrement the refcount. If we're the last ones the object will be
     * free, along with its entries.
     */
    uk_store_release_object(UK_MYDEVICE_LIB_ID, obj);
}

/* Registers handler */
UK_EVENT_HANDLER_(UK_STORE_EVENT_RELEASE_OBJECT, handle_release_object);
```

## Static API

The static API allows libraries to define a flat list of entries, ie without parent object.
The hierarchy is then defined as `library/entry[value]`.
Static entries are at compile-time via the `UK_STORE_STATIC_ENTRY` macro.

The static API is a reduced version of the dynamic API as:
  * There is no requirement for dynamic memory allocation.
  * There is no reference counting.
  * uk_store events are not used.
  * Consumers access entries directly, without the need to call `uk_store_acquire_object()`. Calling `uk_store_get_entry()` requires passing `UK_STORE_OBJECT_ID_NONE` to the `object_id` parameter.

The static API is useful in the cases where a library exports a fixed set of entries that do not depend on individual instances.
A common example is aggregate stats, such as those exported by `uk_alloc`.
For more details refer to the implementation in `lib/ukalloc/stats.c`.
