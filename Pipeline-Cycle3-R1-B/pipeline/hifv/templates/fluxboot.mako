<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Flux density bootstrapping and spectral index fitting</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function() {
    // return a function that sets the SPW text field to the given spw
    var createSpwSetter = function(spw) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
        };
    };

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        o.data("callback", createSpwSetter(spw));
    });

    $(".fancybox").fancybox({
        type: 'image',
        prevEffect: 'none',
        nextEffect: 'none',
        loop: false,
        helpers: {
            title: {
                type: 'outside'
            },
            thumbs: {
                width: 50,
                height: 50,
            }
        }
    });
});
</script>

<p>Make a gain table that includes gain and opacity corrections for final amp cal and for flux density bootstrapping.</p>
<p>Fit the spectral index of calibrators with a power-law and put the fit in the model column.</p>

% for ms in summary_plots:
    
    <ul class="thumbnails">
        % for plot in summary_plots[ms]:
            % if os.path.exists(plot.thumbnail):
            <li class="span3">
                <div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="bootstrappedFluxDensities-summary-${ms}">
                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="Model calibrator flux densities"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </img>
                    </a>

                    <div class="caption">
                    	<h4>Model calibrator
                        </h4>

                        <p>Model calibrator flux densities.  Plot of amp vs. freq
                        </p>
                    </div>
                </div>
            </li>
            % endif
        % endfor
    </ul>
    
<table class="table table-bordered table-striped table-condensed"
	   summary="Spectral Indices">
	<caption>Spectral Indices</caption>
        <thead>
	    <tr>
	        <th scope="col" rowspan="2">Source</th>
	        <th scope="col" rowspan="2">Band</th>
		<th scope="col" rowspan="2">Fitted Spectral Index</th>
		<th scope="col" rowspan="2">SNR</th>
	    </tr>

	</thead>
	<tbody>    
    
    % for row in spindex_results[ms]:
    

		<tr>
		        <td>${row['source']}</td>
			<td>${row['band']}</td>
			<td>${row['spix']}</td>
			<td>${row['SNR']}</td>
		</tr>

    % endfor
	</tbody>
    </table>

    
    
       <table class="table table-bordered table-striped table-condensed"
	   summary="Fitting data with a power law">
	<caption>Fitting data with a power law</caption>
        <thead>
	    <tr>
	        <th scope="col" rowspan="2">Frequency [GHz]</th>
	        <th scope="col" rowspan="2">Data</th>
		<th scope="col" rowspan="2">Error</th>
		<th scope="col" rowspan="2">Fitted Data</th>
	    </tr>

	</thead>
	<tbody>   
    
    % for row in weblog_results[ms]:
    

	
		<tr>
		        <td>${row['freq']}</td>
			<td>${row['data']}</td>
			<td>${row['error']}</td>
			<td>${row['fitteddata']}</td>
		</tr>

    % endfor
	</tbody>
    </table>
    

%endfor