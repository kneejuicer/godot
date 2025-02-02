/*************************************************************************/
/*  openxr_interface.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef OPENXR_INTERFACE_H
#define OPENXR_INTERFACE_H

#include "servers/xr/xr_interface.h"
#include "servers/xr/xr_positional_tracker.h"

#include "action_map/openxr_action_map.h"
#include "openxr_api.h"

// declare some default strings
#define INTERACTION_PROFILE_NONE "/interaction_profiles/none"

class OpenXRInterface : public XRInterface {
	GDCLASS(OpenXRInterface, XRInterface);

private:
	OpenXRAPI *openxr_api = nullptr;
	bool initialized = false;
	XRInterface::TrackingStatus tracking_state;

	// At a minimum we need a tracker for our head
	Ref<XRPositionalTracker> head;
	Transform3D head_transform;
	Vector3 head_linear_velocity;
	Vector3 head_angular_velocity;
	Transform3D transform_for_view[2]; // We currently assume 2, but could be 4 for VARJO which we do not support yet

	void _load_action_map();

	struct Action { // An action we've registered with OpenXR
		String action_name; // Name of our action as presented to Godot (can be altered from the action map)
		OpenXRAction::ActionType action_type; // The action type of this action
		RID action_rid; // RID of the action registered with our OpenXR API
	};
	struct ActionSet { // An action set we've registered with OpenXR
		String action_set_name; // Name of our action set
		bool is_active; // If true this action set is active and we will sync it
		Vector<Action *> actions; // List of actions in this action set
		RID action_set_rid; // RID of the action registered with our OpenXR API
	};
	struct Tracker { // A tracker we've registered with OpenXR
		String tracker_name; // Name of our tracker (can be altered from the action map)
		Vector<Action *> actions; // Actions related to this tracker
		Ref<XRPositionalTracker> positional_tracker; // Our positional tracker object that holds our tracker state
		RID tracker_rid; // RID of the tracker registered with our OpenXR API
		RID interaction_profile; // RID of the interaction profile bound to this tracker (can be null)
	};

	Vector<ActionSet *> action_sets;
	Vector<RID> interaction_profiles;
	Vector<Tracker *> trackers;

	ActionSet *create_action_set(const String &p_action_set_name, const String &p_localized_name, const int p_priority);
	void free_action_sets();

	Action *create_action(ActionSet *p_action_set, const String &p_action_name, const String &p_localized_name, OpenXRAction::ActionType p_action_type, const Vector<Tracker *> p_trackers);
	Action *find_action(const String &p_action_name);
	void free_actions(ActionSet *p_action_set);

	Tracker *find_tracker(const String &p_tracker_name, bool p_create = false);
	void link_action_to_tracker(Tracker *p_tracker, Action *p_action);
	void handle_tracker(Tracker *p_tracker);
	void free_trackers();

	void free_interaction_profiles();

	void _set_default_pos(Transform3D &p_transform, double p_world_scale, uint64_t p_eye);

protected:
	static void _bind_methods();

public:
	virtual StringName get_name() const override;
	virtual uint32_t get_capabilities() const override;

	virtual PackedStringArray get_suggested_tracker_names() const override;
	virtual TrackingStatus get_tracking_status() const override;

	bool initialise_on_startup() const;
	virtual bool is_initialized() const override;
	virtual bool initialize() override;
	virtual void uninitialize() override;

	virtual void trigger_haptic_pulse(const String &p_action_name, const StringName &p_tracker_name, double p_frequency, double p_amplitude, double p_duration_sec, double p_delay_sec = 0) override;

	virtual bool supports_play_area_mode(XRInterface::PlayAreaMode p_mode) override;
	virtual XRInterface::PlayAreaMode get_play_area_mode() const override;
	virtual bool set_play_area_mode(XRInterface::PlayAreaMode p_mode) override;

	virtual Size2 get_render_target_size() override;
	virtual uint32_t get_view_count() override;
	virtual Transform3D get_camera_transform() override;
	virtual Transform3D get_transform_for_view(uint32_t p_view, const Transform3D &p_cam_transform) override;
	virtual CameraMatrix get_projection_for_view(uint32_t p_view, double p_aspect, double p_z_near, double p_z_far) override;

	virtual void process() override;
	virtual void pre_render() override;
	bool pre_draw_viewport(RID p_render_target) override;
	virtual Vector<BlitToScreen> post_draw_viewport(RID p_render_target, const Rect2 &p_screen_rect) override;
	virtual void end_frame() override;

	void on_state_ready();
	void on_state_visible();
	void on_state_focused();
	void on_state_stopping();
	void on_pose_recentered();
	void tracker_profile_changed(RID p_tracker, RID p_interaction_profile);

	OpenXRInterface();
	~OpenXRInterface();
};

#endif // !OPENXR_INTERFACE_H
