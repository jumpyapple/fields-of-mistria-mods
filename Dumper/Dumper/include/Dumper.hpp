/**
* @brief A dumper utility for YYToolkit
* @author jumpyapple
* 
* Based on Archie_uwu and Felix implementation.
* 
* Required libraries
* - nlohmann/json
* - pantor/inja
* 
* Need NOMINMAX to be define, otherwise the std::min, std::max will be
* redefined by windows.h and inja will fail to compile.
* 
* Also need access to YYTK's internal.
*/
#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include <fstream>
#include <unordered_set>
#include <string_view>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <inja.hpp>
#include "Dumper_Shared.hpp"

// These static variables are probably different copies for this own mod
// and when other mod is calling.
static std::unordered_set<uint64_t> visited_pointers;
static std::vector<std::tuple<YYTK::RValue, std::string>> queue;

static const std::unordered_set<std::string_view> IGNORING_KEYS = {
		"node_terrain_tile",
		"node_terrain_kind",
		"node_is_room_editor_collision",
		"node_object_id",
		"node_collideable",
		"node_terrain_is_watered",
		"node_rug_parent",
		"node_pathfinding_cost",
		"node_flags",
		"node_force",
		"node_terrain_variant",
		"node_terrain_is_watered",
		"node_rug_id",
		"node_is_room_editor_collision",
		"node_top_left_y",
		"node_top_left_x",
		"node_terrain_ground_kind",
		"node_pathfinding_sitting",
		"node_parent",
		"maximum_item_counts",
		"node_footstep_kind",
		"node_can_jump_over"
};

static const std::string INDEX_TEMPLATE_TEXT = R"(<!DOCTYPE html>
<html lang="en" data-theme="dark">
<head><title>{{ name }}</title><link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bulma@1.0.4/css/bulma.min.css"></head>
<body>
	<div class="container is-fluid">
		<div class="columns">
			<div class="column is-three-quarters">
				<h1 class="title">{% if exists("type") %}[{{ type }}] {% endif %}{{ name }}</h1>
				<section id="header-section">
					<p>{% if exists("type") %}[{{ type }}] {% endif %}{{ name }} = {{ value }}</p>
				</section>
				<section id="boolean-section">
					<h2 class="subtitle">Booleans</h2>
					<div class="content">
						<ul>
## for item in booleans
							<li>{{ item.name }} = {{ item.value }}</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="int64-section">
					<h2 class="subtitle">Int64</h2>
					<div class="content">
						<ul>
## for item in int64s
							<li>{{ item.name }} = {{ item.value }}</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="number-section">
					<h2 class="subtitle">Numbers</h2>
					<div class="content">
						<ul>
## for item in numbers
							<li>{{ item.name }} = {{ item.value }}</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="string-section">
					<h2 class="subtitle">Strings</h2>
					<div class="content">
						<ul>
## for item in strings
							<li>{{ item.name }} = {{ item.value }}</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="ref-section">
					<h2 class="subtitle">Refs</h2>
					<div class="content">
						<ul>
## for item in refs
							<li>{{ item.name }} = {{ item.value }} {% if existsIn(item, "pointer") %} {{ item.pointer }}{% endif %}</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="undefined-section">
					<h2 class="subtitle">Undefined values</h2>
					<div class="content">
						<ul>
## for item in undefineds
							<li>{{ item.name }}</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="struct-section">
					<h2 class="subtitle">Structs</h2>
					<div class="content">
						<ul>
## for item in structs
							<li>
								{% if existsIn(item, "pointer") %}<a href="{{ item.pointer }}.html">{% endif %}
								{{ item.name }} = {{ item.value}} {% if existsIn(item, "pointer") %} {{ item.pointer }}{% endif %}
								{% if existsIn(item, "pointer") %}</a>{% endif %}
							</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="array-section">
					<h2 class="subtitle">Arrays</h2>
					<div class="content">
						<ul>
## for item in arrays
							<li>
								<details>
									<summary>{{ item.name }} = {{ item.value }}</summary>
									<ul>
## for array_item in item.values
										<li>
## if array_item.type == "struct" or array_item.type == "array"
											{% if existsIn(item, "pointer") %}<a href="{{ array_item.pointer }}.html">{% endif %}
												[{{ array_item.type }}] {{ array_item.name }} = {{ array_item.value }}
											{% if existsIn(item, "pointer") %}</a>{% endif %}
## else
											[{{ array_item.type }}] {{ array_item.name }} = {{ array_item.value }}
## endif
										</li>
## endfor
									</ul>
								</details>
							</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="other-section">
					<h2 class="subtitle">Other</h2>
					<div class="content">
						<ul>
## for item in other
							<li>
								{% if existsIn(item, "pointer") %}<a href="{{ item.pointer }}.html">{% endif %}
								[{{ item.type }}] {{ item.name }}
								{% if existsIn(item, "pointer") %}</a>{% endif %}
							</li>
## endfor
						</ul>
					</div>
				</section>
				<section id="method-section">
					<h2 class="subtitle">Methods</h2>
					<div class="content">
						<ul>
## for item in methods
							<li>{{ item.name }} = {{ item.value }}</li>
## endfor
						</ul>
					</div>
				</section>
			</div>
			<div class="column is-one-quarter" style="position: sticky; max-height: 100vh; top: 0;">
				<aside class="menu">
					<a id="dark-btn" href="#">Dark</a> / <a id="light-btn" href="#">Light</a>
					<p class="menu-label">Sections</p>
					<ul class="menu-list">
						<li><a href="#boolean-section">Booleans</a></li>
						<li><a href="#int64-section">Int64</a></li>
						<li><a href="#number-section">Numbers</a></li>
						<li><a href="#string-section">Strings</a></li>
						<li><a href="#ref-section">Refs</a></li>
						<li><a href="#undefined-section">Undefined</a></li>
						<li><a href="#struct-section">Structs</a></li>
						<li><a href="#array-section">Array</a></li>
						<li><a href="#other-section">Other</a></li>
						<li><a href="#method-section">Methods</a></li>
					</ul>
				</aside>
			</div>
		</div>
	</div>
	<script>
		window.addEventListener("load", () => {
			let html_tag = document.getElementsByTagName("html")[0];
			document.getElementById("dark-btn").addEventListener("click", (e) => {
				e.preventDefault();
				html_tag.setAttribute("data-theme", "dark");
			});
			document.getElementById("light-btn").addEventListener("click", (e) => {
				e.preventDefault();
				html_tag.setAttribute("data-theme", "light");
			});

			if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
				html_tag.setAttribute("data-theme", "dark");
			} else {
				html_tag.setAttribute("data-theme", "light");
			}
		});
	</script>
</body>
</html>
)";

namespace Dumper {

	namespace fs = std::filesystem;

	YYTK::CScript* TryLookupScriptByFunctionPointer(IN YYTK::YYTKInterface* g_ModuleInterface, IN YYTK::PFUNC_YYGMLScript ScriptFunction);

	nlohmann::json ToJsonObject(
		IN YYTK::YYTKInterface* g_ModuleInterface,
		std::string name,
		YYTK::RValue value,
		bool should_recurse_array,
		bool dont_queue
	);
}